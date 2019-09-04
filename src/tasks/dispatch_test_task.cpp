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

#include <board_config.hpp>
#include <DispatchQueue.hpp>

abs_time_t _last_time = 0;

void dispatch_test_task(void* args)
{
	auto dispatcher = new DispatchQueue("dummy_q", PriorityLevel::HIGHEST);

	auto func1 = []
	{
		volatile unsigned dummy = 0;
		for (unsigned i = 0; i < 10; ++i)
		{
			dummy++;
		}

		auto now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();
		auto elapsed = now - _last_time;
		_last_time = now;
		// SYS_INFO("Hey I got dispatched on an interval!");
		// SYS_INFO("interval: %llu", elapsed);
	};

	dispatcher->dispatch_on_interval(func1, 2000);

	for(;;)
	{
		// do nothing
		vTaskDelay(1000);
	}
}