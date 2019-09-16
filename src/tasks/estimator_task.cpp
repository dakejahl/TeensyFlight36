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
#include <dispatch_queue/DispatchQueue.hpp>
#include <ComplimentaryFilter.hpp>


void estimator_task(void* args)
{
	messenger::Subscriber<gyro_raw_data_s> gyro_sub;
	messenger::Subscriber<accel_raw_data_s> accel_sub;
	messenger::Subscriber<mag_raw_data_s> mag_sub;

	messenger::Publisher<attitude_euler> attitude_pub;


	auto estimator = new ComplimentaryFilter(0.1);


	for(;;)
	{
		if (accel_sub.updated())
		{
			estimator->collect_sensor_data();
			estimator->apply();

			auto roll = estimator->get_roll();
			auto pitch = estimator->get_pitch();

			// Publish for our stream
			attitude_euler rpy;
			rpy.roll = roll;
			rpy.pitch = pitch;
			attitude_pub.publish(rpy);
		}

		vTaskDelay(1);
	}
}