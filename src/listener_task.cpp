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
#include <PublishSubscribe.hpp>
#include <DispatchQueue.hpp>

void listener_task(void* args)
{
	// auto accel_sub = new messenger::Subscriber<accel_raw_data_s>();
	messenger::Subscriber<accel_raw_data_s> accel_sub;

	auto dispatcher = new DispatchQueue("dummy_q");

	auto func = []
	{
		volatile unsigned dummy = 0;
		for (unsigned i = 0; i < 1000; ++i)
		{
			dummy++;
		}
	};

	for(;;)
	{
		accel_raw_data_s data;

		if (accel_sub.updated())
		{
			data = accel_sub.get();
			SYS_INFO("listener_task: got some data");
			dispatcher->dispatch(func);
		}
		else
		{
			SYS_INFO("listener_task: no data available");
		}

		vTaskDelay(100);
	}
}