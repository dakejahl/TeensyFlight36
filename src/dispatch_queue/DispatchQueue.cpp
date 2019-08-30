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

#include <timers/Time.hpp>

DispatchQueue::DispatchQueue(const std::string name, const size_t stack_size,
							const PriorityLevel priority)
	: _name(name)
{
	_mutex = xSemaphoreCreateRecursiveMutex();

	SYS_INFO("DispatchQueue: %s", _name.c_str());

	xTaskCreate(reinterpret_cast<void(*)(void*)>(
				BOUNCE(DispatchQueue, dispatch_thread_handler)),
				_name.c_str(),
				stack_size,
				reinterpret_cast<void*>(this), // pass in the "this" pointer as pvParameters...why?
				priority,
				&_task_handle);
}

void DispatchQueue::dispatch(const fp_t& work)
{
	xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
	_queue.push(work);
	xSemaphoreGiveRecursive(_mutex);

	// Wake up worker thread
	xTaskNotifyGive(_task_handle);
	return;
}

void DispatchQueue::dispatch(fp_t&& work)
{
	xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
	_queue.push(std::move(work));
	xSemaphoreGiveRecursive(_mutex);

	// Wake up worker thread
	xTaskNotifyGive(_task_handle);
	return;
}

void DispatchQueue::interval_dispatch_notify_ready()
{
	// Signal to the dispatcher that an interval item is ready to run
	_interval_item_ready = true;

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// Wake up worker thread
	vTaskNotifyGiveFromISR(_task_handle, &xHigherPriorityTaskWoken);

	// If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
	// should be performed to ensure the interrupt returns directly to the highest
	// priority task.
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void DispatchQueue::dispatch_on_interval(const fp_t& work, abs_time_t interval_ms)
{
	xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);

	if (time::DispatchTimer::Instance() == nullptr)
	{
		time::DispatchTimer::Instantiate(this);
	}

	// Push item into an interval queue -- must disable interrupts since this is shared w/ timer isr.
	IntervalWork item;

	auto now = time::DispatchTimer::Instance()->get_absolute_time_us();

	item.work = work;
	item.interval_ms = interval_ms;
	item.next_deadline_us = now;

	taskENTER_CRITICAL();

	_interval_list.push_back(item);

	interval_dispatch_invoke_scheduler();

	taskEXIT_CRITICAL();

	xSemaphoreGiveRecursive(_mutex);
}
void DispatchQueue::dispatch_on_interval(fp_t&& work, abs_time_t interval_ms)
{
	xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);

	if (time::DispatchTimer::Instance() == nullptr)
	{
		// Initialize the timer if we haven't already.
		time::DispatchTimer::Instantiate(this);
	}

	// Push item into an interval queue -- must disable interrupts since this is shared w/ timer isr.
	IntervalWork item;

	auto now = time::DispatchTimer::Instance()->get_absolute_time_us();

	item.work = work;
	item.interval_ms = interval_ms;
	item.next_deadline_us = now;

	taskENTER_CRITICAL();

	_interval_list.push_back(std::move(item));

	interval_dispatch_invoke_scheduler();

	taskEXIT_CRITICAL();

	xSemaphoreGiveRecursive(_mutex);
}

// MUST ONLY BE CALLED WHEN INTERRUPTS ARE DISABLED
void DispatchQueue::interval_dispatch_invoke_scheduler(void)
{
	abs_time_t deadline_us = time::MAX_TIME;
	for (auto it = _interval_list.begin(); it != _interval_list.end(); ++it)
	{
		auto& item = *it;
		auto item_deadline_us = item.next_deadline_us;

		if (item_deadline_us < deadline_us)
		{
			deadline_us = item_deadline_us;
			_interval_list._next = it; // Point _next at the iterator of the next item to run.
			// NOTE: this is unsafe if something comes and messes with our list, we must
			// make the assumption that this will not happen.
		}

		// Check if we need to reschedule immediately
		auto current_time_us = time::DispatchTimer::Instance()->get_absolute_time_us();

		if (current_time_us >= deadline_us)
		{
			// Flag as ready
			_interval_item_ready = true;
		}
		// All ready to run items have been run. Reenable DispatchTimer scheduling and set the next deadline.
		else
		{
			time::DispatchTimer::Instance()->schedule_next_deadline_us(deadline_us);
		}
	}
}

void DispatchQueue::dispatch_thread_handler(void)
{
	xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);

	abs_time_t start_time = time::DispatchTimer::Instance()->get_absolute_time_us();

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
				_interval_item_ready = false;

				// NOTE: This line right here effectively disables scheduling from the DispatchTimer. Scheduling
				// will be reenabled once all the ready to run interval items become blocked again. See interval_dispatch_invoke_scheduler().
				// TODO: make this logic less shitty... this is quite hard to see...
				time::DispatchTimer::Instance()->disable_scheduling(); // to prevent race conditions

				taskEXIT_CRITICAL();

				work();

				// NOTE: Reschedule the item for the next interval only after work has completed
				taskENTER_CRITICAL();

				// Reschedule based on entrance time to ensure interval precision
				item.next_deadline_us = start_time + item.interval_ms * MICROS_PER_MILLI;

				interval_dispatch_invoke_scheduler();

				taskEXIT_CRITICAL();
			}
			else
			// Otherwise we service the async queue
			{
				auto work = std::move(_queue.front());
				_queue.pop();

				xSemaphoreGiveRecursive(_mutex);

				// Run function
				work();

				xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
			}
		}
		// Our queue is empty -- go to sleep until we have work.
		else if(!_should_exit)
		{
			xSemaphoreGiveRecursive(_mutex);

			// Wait for new work - clear flags on exit
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

			// Get start time such that we can reschedule accurately
			start_time = time::DispatchTimer::Instance()->get_absolute_time_us();

			// We are awake! Time to do some work...
			xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
		}
		else
		{
			// It's time to exit!
		}

	} while (!_should_exit);

	// We were holding the mutex after we woke up
	xSemaphoreGiveRecursive(_mutex);

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

	join_worker_thread();

    // Cleanup mutex
    vSemaphoreDelete(_mutex);
}

void DispatchQueue::join_worker_thread(void)
{
	eTaskState state;

	do {
		// Keep signalling the task to wake up as long as it has not died
		xTaskNotifyGive(_task_handle);

		// Give it some time...
		vTaskDelay(10);

		// If not dead, keeping looping/signalling
		state = eTaskGetState(_task_handle);
	} while (state != eDeleted);
}