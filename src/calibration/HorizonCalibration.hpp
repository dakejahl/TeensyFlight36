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

class HorizonCalibration
{
public:
	void calibrate(void)
	{
		// We will sit for 3 seconds and collect data at 10Hz
		auto start = time::HighPrecisionTimer::Instance()->get_absolute_time_us();
		abs_time_t run_time = 3000000; // 3 seconds
		auto now = start;

		unsigned num_samples = 0;
		while (now < start + run_time)
		{
			if (_attitude_sub.updated())
			{
				auto data = _attitude_sub.get();

				float roll = data.roll;
				float pitch = data.pitch;

				// gyro_data->push_back(data);
				_roll_accumulate += roll;
				_pitch_accumulate += pitch;
				num_samples++;
			}

			vTaskDelay(100); // run at 10hz -- 30 samples
			now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();
		}

		auto roll_offset = _roll_accumulate / num_samples;
		auto pitch_offset = _pitch_accumulate / num_samples;

		// Offset is the average value of the measurement
		SYS_INFO("roll_offset: %f", roll_offset);
		SYS_INFO("pitch_offset: %f", pitch_offset);
	}

private:
	messenger::Subscriber<attitude_euler_s> _attitude_sub;
	double _roll_accumulate = 0;
	double _pitch_accumulate = 0;
};