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

#include <event_groups.h>
#include <semphr.h>

#include <functional>
#include <queue>
#include <list>
#include <string>
#include <vector>

#define BOUNCE(c,m) bounce<c, decltype(&c::m), &c::m>

// Bounce for C++ --> C function callbacks
template<class T, class Method, Method m, class ...Params>
static auto bounce(void *priv, Params... params) ->
		decltype(((*reinterpret_cast<T *>(priv)).*m)(params...))
{
	return ((*reinterpret_cast<T *>(priv)).*m)(params...);
}

typedef std::function<void(void)> fp_t;

struct IntervalWork
{
	fp_t work;
	abs_time_t interval_ms;
	abs_time_t next_deadline_us;
};

struct IntervalList
{
	std::list<IntervalWork> _items;
	std::list<IntervalWork>::iterator _next; // holds the iterator index for the next item to run

	std::list<IntervalWork>::iterator begin() { return _items.begin(); };
	std::list<IntervalWork>::iterator end() { return _items.end(); };
	void push_back(const IntervalWork& value) { _items.push_back(value); };
	void push_back(IntervalWork&& value) { _items.push_back(value); };
};

//-------------------- Impl --------------------//

class DispatchQueue
{
public:
	DispatchQueue(const std::string name, const size_t stack_size = 1024,
		const PriorityLevel priority = ::LOW_PRI_Q);

	~DispatchQueue(void);

	void dispatch(const fp_t& work);
	void dispatch(fp_t&& work);

	void dispatch_on_interval(const fp_t& work, unsigned interval_ms);
	void dispatch_on_interval(fp_t&& work, unsigned interval_ms);

	void interval_dispatch_notify_ready(void);
	void interval_dispatch_update_iterator(void);

private:
	void dispatch_thread_handler(void);

	void join_worker_thread(void);

	std::string _name;
	std::queue<fp_t> _queue; // holds async items
	IntervalList _interval_list; // holds interval items

	TaskHandle_t _task_handle;
	SemaphoreHandle_t _mutex;

	volatile bool _interval_item_ready = false;

	bool _should_exit = false;
};