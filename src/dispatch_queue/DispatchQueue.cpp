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


DispatchQueue::DispatchQueue(const std::string name, const PriorityLevel priority,
							const size_t stack_size)
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

	// Intialize dispatch timer for interval work -- must come after the DispatchQueue task handle has been created!
	_interval_dispatcher = new IntervalDispatchScheduler(this);
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

void DispatchQueue::notify()
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// Wake up worker thread
	vTaskNotifyGiveFromISR(_task_handle, &xHigherPriorityTaskWoken);

	// If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
	// should be performed to ensure the interrupt returns directly to the highest
	// priority task.
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void DispatchQueue::dispatch_on_interval(const fp_t& work, abs_time_t interval)
{
	xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);

	// Push item into an interval queue -- must disable interrupts since this is shared w/ timer isr.

	taskENTER_CRITICAL();

	_interval_dispatcher->disable_scheduler();
	_interval_dispatcher->add_item(work, interval);
	_interval_dispatcher->invoke_scheduler();
	_interval_dispatcher->enable_scheduler();

	taskEXIT_CRITICAL();

	xSemaphoreGiveRecursive(_mutex);
}
void DispatchQueue::dispatch_on_interval(fp_t&& work, abs_time_t interval)
{
	xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);

	// Push item into an interval queue -- must disable interrupts since this is shared w/ timer isr.

	taskENTER_CRITICAL();

	_interval_dispatcher->disable_scheduler();
	_interval_dispatcher->add_item(std::move(work), interval);
	_interval_dispatcher->invoke_scheduler();
	_interval_dispatcher->enable_scheduler();

	taskEXIT_CRITICAL();

	xSemaphoreGiveRecursive(_mutex);
}

void DispatchQueue::dispatch_thread_handler(void)
{
	xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);

	do
	{
		// Do work while the queue is not empty OR if we have an interval item ready to run
		// TODO: clean up -- break apart and simplify logic
		if(!_should_exit && (!_queue.empty() || _interval_dispatcher->item_ready()))
		{
			// First we check to see if there's an interval item ready to run
			if (_interval_dispatcher->item_ready())
			{
				taskENTER_CRITICAL();

				// Disable the scheduler
				_interval_dispatcher->disable_scheduler();

				// Grab the next ready to run interval item
				auto* item = _interval_dispatcher->get_ready_item();

				// Reschedule based on entrance time to ensure interval precision
				_interval_dispatcher->reschedule_item(item);
				_interval_dispatcher->invoke_scheduler();
				_interval_dispatcher->enable_scheduler();

				taskEXIT_CRITICAL();

				item->work();
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

//----- INTERVAL DISPATCHER -----//
void IntervalDispatchScheduler::add_item(const fp_t& work, abs_time_t interval)
{
	auto deadline_now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();

	_interval_list.push_back(IntervalWork(work, interval, deadline_now));
}

void IntervalDispatchScheduler::add_item(fp_t&& work, abs_time_t interval)
{
	auto deadline_now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();

	_interval_list.push_back(IntervalWork(std::move(work), interval, deadline_now));
}

void IntervalDispatchScheduler::reschedule_item(IntervalWork* item)
{
	auto now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();

	item->deadline = now + item->interval;
}

// MUST ONLY BE CALLED WHEN INTERRUPTS ARE DISABLED
void IntervalDispatchScheduler::invoke_scheduler(void)
{
	if (_interval_list.empty())
	{
		return;
	}

	if (_next_item_to_run == nullptr)
	{
		_next_item_to_run = &(*_interval_list.begin());
	}

	auto now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();

	// Iterate over the list of interval items and mark if ready
	for (auto &it : _interval_list)
	{
		if (it.deadline < _next_item_to_run->deadline)
		{
			_next_item_to_run = &it;
		}
	}

	// If the deadline has already passed, raise _an_item_is_ready flag
	if (now >= _next_item_to_run->deadline)
	{
		_an_item_is_ready = true;
	}
}

void IntervalDispatchScheduler::disable_scheduler(void)
{
	_an_item_is_ready = false;
	// Tell timer ISR to not call the scheduler callback
	time::HighPrecisionTimer::Instance()->disable_callback();
}

void IntervalDispatchScheduler::enable_scheduler(void)
{
	// Tell timer ISR to not call the scheduler callback
	time::HighPrecisionTimer::Instance()->enable_callback();
}


IntervalDispatchScheduler::IntervalDispatchScheduler(DispatchQueue* dispatcher)
	: _dispatcher(dispatcher)

{
	taskENTER_CRITICAL();

	if (time::HighPrecisionTimer::Instance() == nullptr)
	{
		time::HighPrecisionTimer::Instantiate();
	}

	// Register the scheduling callback with the timer instance
	time::HighPrecisionTimer::Instance()->register_overflow_callback<IntervalDispatchScheduler>(this);

	taskEXIT_CRITICAL();
}

void IntervalDispatchScheduler::timer_overflow_callback(void)
{
	// Notify the dispatcher that an interval item is ready to run
	if (_dispatcher != nullptr && _next_item_to_run != nullptr)
	{
		auto now = time::HighPrecisionTimer::Instance()->get_absolute_time_us_from_isr();

		if (now >= _next_item_to_run->deadline)
		{
			// This function will unblock the DispatchQueue to run an Interval item
			_an_item_is_ready = true;
			_dispatcher->notify();
		}
	}
}
