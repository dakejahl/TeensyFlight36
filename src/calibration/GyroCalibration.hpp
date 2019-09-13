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
#include <Messenger.hpp>
#include <Time.hpp>

class GyroCalibration
{
public:
	void calibrate(void)
	{
		// We will sit for 3 seconds and collect data at 10Hz (gyro runs at 1khz)
		auto start = time::HighPrecisionTimer::Instance()->get_absolute_time_us();
		abs_time_t run_time = 3000000; // 3 seconds
		auto now = start;

		unsigned num_samples = 0;
		while (now < start + run_time)
		{
			if (gyro_sub.updated())
			{
				auto data = gyro_sub.get();
				// gyro_data->push_back(data);
				_accumulate_x += data.x;
				_accumulate_y += data.y;
				_accumulate_z += data.z;
				num_samples++;
			}

			vTaskDelay(100); // run at 10hz -- 30 samples
			now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();
		}

		auto x_offset = _accumulate_x / num_samples;
		auto y_offset = _accumulate_y / num_samples;
		auto z_offset = _accumulate_z / num_samples;

		// Offset is the average value of the measurement
		SYS_INFO("gyro_offset_x: %f", x_offset);
		SYS_INFO("gyro_offset_y: %f", y_offset);
		SYS_INFO("gyro_offset_z: %f", z_offset);
	}

private:
	messenger::Subscriber<gyro_raw_data_s> gyro_sub;

	double _accumulate_x = 0;
	double _accumulate_y = 0;
	double _accumulate_z = 0;

};