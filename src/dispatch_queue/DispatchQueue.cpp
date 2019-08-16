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

DispatchQueue::DispatchQueue(const std::string name, const size_t stack_size,
							const PriorityLevel priority, const uint8_t num_threads)
	: _name(name)
	, _threads(num_threads)
{
	// NOTE: if a task successfully ‘takes’ the same mutex 5 times then
	// the mutex will not be available to any other task until it has
	// also ‘given’ the mutex back exactly five times.
	_mutex = xSemaphoreCreateRecursiveMutex(); // cannot be used in ISR.
	// assert(mutex_ != NULL && "Failed to create mutex!");

	_notify_flags = xEventGroupCreate();
	// assert(notify_flags_ != NULL && "Failed to create event group!");

	// Initialize our thread(s)
	for(size_t i = 0; i < _threads.size(); i++)
	{
		// TODO: get std::to_string() to work.
		char buf[sizeof(size_t)];
		itoa(i,buf, 10);
		_threads[i].name = std::string("Dispatch Thread #" + std::string(buf));

		println(_threads[i].name);

		BaseType_t status = xTaskCreate(reinterpret_cast<void(*)(void*)>(
								BOUNCE(DispatchQueue, dispatch_thread_handler)),
								_threads[i].name.c_str(),
								stack_size,
								reinterpret_cast<void*>(this), // pass in the "this" pointer as pvParameters...why?
								priority,
								&_threads[i].thread); // pass the TaskHandle in...why?
		(void)status;
		// assert(status == pdPASS && "Failed to create thread!");
	}
}

void DispatchQueue::dispatch(const fp_t& work)
{
	BaseType_t status = xSemaphoreTakeRecursive(_mutex, portMAX_DELAY/*Blocking...*/);
	(void)status;
	// assert(status == pdTRUE && "Failed to lock mutex!");
	_queue.push(work);
	status = xSemaphoreGiveRecursive(_mutex);
	// assert(status == pdTRUE && "Failed to unlock mutex!");

	// Wake up worker thread
	xEventGroupSetBits(_notify_flags, Event::DISPATCH_WAKE);
	return;
}

void DispatchQueue::dispatch_thread_handler(void)
{
	BaseType_t status = xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
	// assert(status == pdTRUE && "Failed to lock mutex!");

	do
	{
		// Do work while the queue is not empty.
		if(!_should_exit && !_queue.empty())
		{
			auto work = std::move(_queue.front());
			_queue.pop();

			status = xSemaphoreGiveRecursive(_mutex);
			// assert(status == pdTRUE && "Failed to unlock mutex!");

			this->println("Hello Mr. Jake");
			work();

			status = xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
			// assert(status == pdTRUE && "Failed to lock mutex!");
		}
		// Our queue is empty -- go to sleep until we have work.
		else if(!_should_exit)
		{
			status = xSemaphoreGiveRecursive(_mutex);
			// assert(status == pdTRUE && "Failed to unlock mutex!");

			// Wait for new work - clear flags on exit
			xEventGroupWaitBits(_notify_flags, Event::DISPATCH_WAKE, pdTRUE, pdFALSE, portMAX_DELAY);

			// We are awake! Time to do some work...
			status = xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
			// assert(status == pdTRUE && "Failed to lock mutex!");
		}
		else
		{
			// It's time to exit!
		}

	} while (!_should_exit);

	// We were holding the mutex after we woke up
	status = xSemaphoreGiveRecursive(_mutex);
	// assert(status == pdTRUE && "Failed to unlock mutex!");

	// Inidcate to DTOR that thread is cleaned up
	status = xEventGroupSetBits(_notify_flags, Event::DISPATCH_EXIT);
	(void)status;
	// assert(status == pdTRUE && "Failed to set event flags!");

	// NOTE: The idle task is responsible for freeing the RTOS kernel allocated
	// memory from tasks that have been deleted. It is therefore important that
	// the idle task is not starved of microcontroller processing time if your
	// application makes any calls to vTaskDelete(). Memory allocated by the
	// task code is not automatically freed, and should be freed before the task
	// is deleted.

	vTaskDelete(NULL); // NULL causes calling task to be deleted
 	return;
}

void DispatchQueue::println(std::string words)
{
	// TODO: Serial is not thread safe... add a globle printer
	// function (printf!) guarded with a mutex.
	Serial.println(words.c_str());
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

		// We continually loop sending the exit signal until we detect that ALL threads have died
		do {
			// Signal wake - check exit flag
			xEventGroupSetBits(_notify_flags, Event::DISPATCH_WAKE);

			// Wait until a thread signals exit. Timeout is acceptable.
			xEventGroupWaitBits(_notify_flags, Event::DISPATCH_EXIT, pdTRUE, pdFALSE, pdMS_TO_TICKS(10)); // wait 10 ms

			// If it was not _thread[i], that is ok, but we will loop around
			// until _threads[i] has exited
			state = eTaskGetState(_threads[i].thread);
		} while (state != eDeleted);

		_threads[i].name.clear();
	}
}