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

#include <Arduino.h>

#include <freertos/include/FreeRTOS.h>
#include <freertos/include/task.h>
#include <freertos/include/event_groups.h>
#include <freertos/include/semphr.h>

#include <functional>
#include <queue>
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

struct freertos_thread_t
{
	TaskHandle_t thread;
	std::string name;
};

enum PriorityLevel : uint8_t
{
	LOWEST = 0,
	LOW_PRI_Q = 1,
	HI_PRI_Q = 2,
	HIGHEST = configMAX_PRIORITIES,
};

enum Event : uint8_t
{
	DISPATCH_WAKE = 0x01,
	DISPATCH_EXIT = 0x02,
};

//-------------------- Impl --------------------//

class DispatchQueue
{
public:
	DispatchQueue(const std::string name, const size_t stack_size = 1024,
		const PriorityLevel priority = ::LOW_PRI_Q, const uint8_t num_threads = 1);

	~DispatchQueue(void);

	void dispatch(const fp_t& work);

private:
	void dispatch_thread_handler(void);

	void join_worker_threads(void);

	void println(std::string words);

	std::string _name;
	std::vector<freertos_thread_t> _threads;
	std::queue<fp_t> _queue;

	SemaphoreHandle_t _mutex;
	// FreeRTOS event flags - like condition variable, used for waking queue threads
	EventGroupHandle_t _notify_flags;

	bool _should_exit = false;
};