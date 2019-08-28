// MIT License

// Copyright (c) 2019 Jacob Dahl

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "DispatchQueue.hpp"

#include <Time.hpp>

DispatchQueue::DispatchQueue(const std::string name, const size_t stack_size,
							const PriorityLevel priority, const uint8_t num_threads)
	: _name(name)
	, _threads(num_threads)
{
	// NOTE: if a task successfully ‘takes’ the same mutex 5 times then
	// the mutex will not be available to any other task until it has
	// also ‘given’ the mutex back exactly five times.
	_mutex = xSemaphoreCreateRecursiveMutex(); // cannot be used in ISR.
	_notify_flags = xEventGroupCreate();

	// Initialize our thread(s)
	for(size_t i = 0; i < _threads.size(); i++)
	{
		// TODO: get std::to_string() to work.
		char buf[sizeof(size_t)];
		itoa(i,buf, 10);
		_threads[i].name = std::string("Dispatch Thread #" + std::string(buf));

		SYS_INFO(_threads[i].name.c_str());

		xTaskCreate(reinterpret_cast<void(*)(void*)>(
					BOUNCE(DispatchQueue, dispatch_thread_handler)),
					_threads[i].name.c_str(),
					stack_size,
					reinterpret_cast<void*>(this), // pass in the "this" pointer as pvParameters...why?
					priority,
					&_threads[i].thread); // pass the TaskHandle in...why?
	}
}

void DispatchQueue::dispatch(const fp_t& work)
{
	xSemaphoreTakeRecursive(_mutex, portMAX_DELAY/*Blocking...*/);
	_queue.push(work);
	xSemaphoreGiveRecursive(_mutex);

	// Wake up worker thread
	xEventGroupSetBits(_notify_flags, Event::DISPATCH_WAKE);
	return;
}

void DispatchQueue::dispatch(fp_t&& work)
{
	xSemaphoreTakeRecursive(_mutex, portMAX_DELAY/*Blocking...*/);
	_queue.push(std::move(work));
	xSemaphoreGiveRecursive(_mutex);

	// Wake up worker thread
	xEventGroupSetBits(_notify_flags, Event::DISPATCH_WAKE);
	return;
}

void DispatchQueue::interval_dispatch_notify_ready()
{
	// Signal to the dispatcher that an interval item is ready to run
	_interval_item_ready = true;

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// Wake up worker thread
	xEventGroupSetBitsFromISR(_notify_flags, Event::DISPATCH_WAKE, &xHigherPriorityTaskWoken);
}

void DispatchQueue::dispatch_on_interval(const fp_t& work, unsigned interval_ms)
{
	xSemaphoreTakeRecursive(_mutex, portMAX_DELAY/*Blocking...*/);

	if (time::DispatchTimer::Instance() == nullptr)
	{
		// Initialize the timer if we haven't already.
		time::DispatchTimer::Instantiate(this);
	}

	// Push item into an interval queue -- must disable interrupts since this is shared w/ timer isr.
	IntervalWork item;

	auto now = time::DispatchTimer::Instance()->get_absolute_time_ms();

	item.work = work;
	item.interval_ms = interval_ms;
	item.next_deadline_ms = now + interval_ms;

	taskENTER_CRITICAL();

	_interval_list.push_back(item);

	interval_dispatch_update_iterator();

	taskEXIT_CRITICAL();

	xSemaphoreGiveRecursive(_mutex);
}
void DispatchQueue::dispatch_on_interval(fp_t&& work, unsigned interval_ms)
{
	xSemaphoreTakeRecursive(_mutex, portMAX_DELAY/*Blocking...*/);

	if (time::DispatchTimer::Instance() == nullptr)
	{
		// Initialize the timer if we haven't already.
		time::DispatchTimer::Instantiate(this);
	}

	// Push item into an interval queue -- must disable interrupts since this is shared w/ timer isr.
	IntervalWork item;

	auto now = time::DispatchTimer::Instance()->get_absolute_time_ms();

	item.work = work;
	item.interval_ms = interval_ms;
	item.next_deadline_ms = now + interval_ms;

	taskENTER_CRITICAL();

	_interval_list.push_back(std::move(item));

	interval_dispatch_update_iterator();

	taskEXIT_CRITICAL();

	xSemaphoreGiveRecursive(_mutex);
}

// MUST ONLY BE CALLED WHEN INTERRUPTS ARE DISABLED
void DispatchQueue::interval_dispatch_update_iterator(void)
{
	unsigned counter = 0;

	abs_time_t deadline_ms = time::MAX_TIME;
	for (auto it = _interval_list.begin(); it != _interval_list.end(); ++it)
	{
		auto& item = *it;
		auto item_deadline_ms = item.next_deadline_ms;

		if (item_deadline_ms < deadline_ms)
		{
			deadline_ms = item_deadline_ms;
			_interval_list._next = it; // Point _next at the iterator of the next item to run.
			// NOTE: this is unsafe if something comes and messes with our list, we must
			// make the assumption that this will not happen.
		}
		counter++;
	}

	time::DispatchTimer::Instance()->set_next_deadline_ms(deadline_ms);
}

void DispatchQueue::dispatch_thread_handler(void)
{
	BaseType_t status = xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);

	do
	{
		// Do work while the queue is not empty OR if we have an interval item ready to run
		// TODO: clean up -- break apart and simplify logic
		if(!_should_exit && (!_queue.empty() || _interval_item_ready))
		{
			// First we check to see if there's an interval item ready to run
			if (_interval_item_ready)
			{
				taskENTER_CRITICAL();

				// point _next at the next ready to run item
				auto& item = *_interval_list._next; // dereference the iterator
				auto work = item.work;

				// Reschedule the item for the next interval
				auto now = time::DispatchTimer::Instance()->get_absolute_time_ms();

				item.next_deadline_ms = now + item.interval_ms;

				_interval_item_ready = false;

				interval_dispatch_update_iterator();

				taskEXIT_CRITICAL();

				auto start_time = time::PrecisionTimer::Instance()->get_absolute_time_us();

				work();

				auto end_time = time::PrecisionTimer::Instance()->get_absolute_time_us();
				auto elapsed = end_time - start_time;
			}
			else
			// Otherwise we service the async queue
			{
				auto work = std::move(_queue.front());
				_queue.pop();

				status = xSemaphoreGiveRecursive(_mutex);

				// Run function
				work();

				status = xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
			}
		}
		// Our queue is empty -- go to sleep until we have work.
		else if(!_should_exit)
		{
			status = xSemaphoreGiveRecursive(_mutex);

			// Wait for new work - clear flags on exit
			xEventGroupWaitBits(_notify_flags, Event::DISPATCH_WAKE, pdTRUE, pdFALSE, portMAX_DELAY);

			// We are awake! Time to do some work...
			status = xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
		}
		else
		{
			// It's time to exit!
		}

	} while (!_should_exit);

	// We were holding the mutex after we woke up
	status = xSemaphoreGiveRecursive(_mutex);

	// Inidcate to DTOR that thread is cleaned up
	status = xEventGroupSetBits(_notify_flags, Event::DISPATCH_EXIT);
	(void)status;

	// NOTE: The idle task is responsible for freeing the RTOS kernel allocated
	// memory from tasks that have been deleted. It is therefore important that
	// the idle task is not starved of microcontroller processing time if your
	// application makes any calls to vTaskDelete(). Memory allocated by the
	// task code is not automatically freed, and should be freed before the task
	// is deleted.

	vTaskDelete(NULL); // NULL causes calling task to be deleted
 	return;
}

DispatchQueue::~DispatchQueue(void)
{
	_should_exit = true;

	join_worker_threads();

    // Cleanup event flags and mutex
    vEventGroupDelete(_notify_flags);
    vSemaphoreDelete(_mutex);
}

void DispatchQueue::join_worker_threads(void)
{
	for (size_t i = 0; i < _threads.size(); ++i) {
		eTaskState state;

		// We continually loop sending the exit signal until each thread has died
		do {
			// Signal wake - check exit flag
			xEventGroupSetBits(_notify_flags, Event::DISPATCH_WAKE);

			// Wait until a thread signals exit. Timeout is acceptable.
			xEventGroupWaitBits(_notify_flags, Event::DISPATCH_EXIT, pdTRUE, pdFALSE, pdMS_TO_TICKS(10)); // wait 10 ms

			// If not dead, keeping looping/signalling
			state = eTaskGetState(_threads[i].thread);
		} while (state != eDeleted);

		_threads[i].name.clear();
	}
}