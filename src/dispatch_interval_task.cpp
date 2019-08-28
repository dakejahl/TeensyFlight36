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
#include <Messenger.hpp>


void dispatch_interval_task(void* args)
{
	// auto accel_pub = new messenger::Publisher<accel_raw_data_s>();
	auto dispatcher = new DispatchQueue("dummy_q");
	vTaskDelay(1000);

	auto func = []
	{
		volatile unsigned dummy = 0;
		for (unsigned i = 0; i < 1000; ++i)
		{
			dummy++;
		}

		SYS_INFO("Hey I got dispatched on an interval!");
	};

	auto udder_func = []
	{
		SYS_INFO("jus chillin");
	};

	dispatcher->dispatch_on_interval(func, 1);

	for(;;)
	{
		// We do nothing...
		vTaskDelay(1000);
		dispatcher->dispatch(udder_func);
	}
}