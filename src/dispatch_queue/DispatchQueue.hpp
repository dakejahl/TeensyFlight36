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

#pragma once

#include <board_config.hpp>
#include <timers/Time.hpp>

#include <event_groups.h>
#include <semphr.h>

#include <queue>
#include <list>
#include <string>
#include <vector>

struct IntervalWork
{
	IntervalWork()
		: work(nullptr)
		, interval(0)
		, deadline(time::MAX_TIME)
	{}
	IntervalWork(const fp_t& work, abs_time_t interval, abs_time_t deadline)
		: work(work)
		, interval(interval)
		, deadline(deadline)
	{}
	IntervalWork(fp_t&& work, abs_time_t interval, abs_time_t deadline)
		: work(std::move(work))
		, interval(interval)
		, deadline(deadline)
	{}

	fp_t work;
	abs_time_t interval;
	abs_time_t deadline;
};

//-------------------- Impl --------------------//

class IntervalDispatchScheduler;

class DispatchQueue
{
public:
	DispatchQueue(const std::string name, const PriorityLevel priority = ::HIGHEST,
		const size_t stack_size = 1024);

	~DispatchQueue(void);

	void dispatch(const fp_t& work);
	void dispatch(fp_t&& work);

	void dispatch_on_interval(const fp_t& work, abs_time_t interval);
	void dispatch_on_interval(fp_t&& work, abs_time_t interval);

	void notify(void);

private:
	void dispatch_thread_handler(void);
	void join_worker_thread(void);

	std::string _name;
	std::queue<fp_t> _queue; // holds async items

	IntervalDispatchScheduler* _interval_dispatcher;

	TaskHandle_t _task_handle;
	SemaphoreHandle_t _mutex;

	volatile bool _should_exit = false;
};

class IntervalDispatchScheduler
{
public:
	IntervalDispatchScheduler(DispatchQueue* dispatcher);

	void add_item(const fp_t& work, abs_time_t interval);
	void add_item(fp_t&& work, abs_time_t interval);

	void timer_overflow_callback(void);

	// void disable_scheduling(void);
	void reschedule_item(IntervalWork* item);
	void invoke_scheduler(void);
	void enable_scheduler(void);
	void disable_scheduler(void);

	bool item_ready(void) { return _interval_list.size() && _an_item_is_ready; };
	IntervalWork* get_ready_item(void) { return _next_item_to_run; };

	IntervalWork& dispatch_get_next_item(void);

private:

	std::list<IntervalWork> _interval_list; // holds interval items

	IntervalWork* _next_item_to_run = nullptr;
	volatile bool _an_item_is_ready = false;

	static IntervalDispatchScheduler* _instance;
	DispatchQueue* _dispatcher = nullptr;
};