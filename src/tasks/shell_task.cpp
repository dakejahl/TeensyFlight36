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
#include <vector>
#include <AccelCalibration.hpp>

std::string buffer;
std::string GYRO_CAL = "gyro cal";
std::string ACCEL_CAL = "accel cal";


void evaluate_user_command(void);
void calibrate_gyro(void);
void calibrate_accel(void);


void shell_task(void* args)
{
	// This task will poll the USB interface for user commands:
	// - gyro calibration, let sit still and measure offsets on all 3 axis (and noise value for ekf?)
	// - accel calibration (6 sides), find offsets and scales for all 3 axis
	// - mag calibration (rotate A LOT, find max/min on all 3 axis, calculate offsets and scale factor)

	char byte = 0;

	for(;;)
	{
		// If we get something at the serial port, read it and print it
		// Serial port PC side sends strings at a time terminated with a LF
		if (Serial.available())
		{
			byte = Serial.read();

			// We got some data -- First check if its a CR
			if (byte == 10) // line feed
			{
				// Evaluate buffer
				evaluate_user_command();
				buffer.clear();
			}
			else
			{
				// Add the character to our buffer, when we recieve a CR (enter) we will compare the buffer
				// against the list of known commands.
				buffer.push_back(byte);
			}
		}
		else
		{
			// sleep while we have no data
			vTaskDelay(100); // run at 10hz
		}
	}
}

// Put user command functions here
void evaluate_user_command(void)
{
	if (buffer == GYRO_CAL)
	{
		// Run the gyro calibration
		SYS_INFO("Calibrating gyro");
		calibrate_gyro();
	}
	else if (buffer == ACCEL_CAL)
	{
		SYS_INFO("Calibrating accel");
		calibrate_accel();
	}
}

void calibrate_gyro(void)
{
	messenger::Subscriber<gyro_raw_data_s> gyro_sub;

	// We will sit for 3 seconds and collect data at 10Hz (gyro runs at 1khz)
	auto start = time::HighPrecisionTimer::Instance()->get_absolute_time_us();
	abs_time_t run_time = 3000000; // 3 seconds

	auto now = start;

	// Buffer of data points
	auto gyro_data = new std::vector<gyro_raw_data_s>();

	while (now < start + run_time)
	{
		if (gyro_sub.updated())
		{
			auto data = gyro_sub.get();
			gyro_data->push_back(data);
		}

		vTaskDelay(100); // run at 10hz -- 30 samples
		now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();
	}

	float x_sum = 0;
	float y_sum = 0;
	float z_sum = 0;

	for (auto& g : *gyro_data)
	{
		x_sum += g.x;
		y_sum += g.y;
		z_sum += g.z;
	}

	auto x_offset = x_sum / gyro_data->size();
	auto y_offset = y_sum / gyro_data->size();
	auto z_offset = z_sum / gyro_data->size();

	// Offset is the average value of the measurement
	SYS_INFO("gyro_offset_x: %f", x_offset);
	SYS_INFO("gyro_offset_y: %f", y_offset);
	SYS_INFO("gyro_offset_z: %f", z_offset);

	// We should have a buffer of ~30 data points X 24 bytes per data point == 720B increased heap

	// time to clean up
	delete gyro_data;
}

// For this one we want to put the sensor on each side (6 sides) and measure the static reading.
// The static reading should just be the gravity field vector "g" which we know is 9.81 m/s2.
// We will just look for the value on an axis to be greater than 9, at which point we will
// just delay 2 seconds, and then sample for 3 seconds at 10hz (30 samples)
void calibrate_accel(void)
{
	AccelCalibration calibration;

	while (!calibration.all_sides_complete())
	{
		auto side = calibration.get_next_side_to_calibrate();

		calibration.calibrate(side);
	}

	// The gravity vector has been measured on each side, we now want to find the offsets and scales.
	calibration.calculate_offsets_and_scales();

	// TODO:
	// estimation::Estimator::Instance()->update_gyro_offset() ... or something like that
	// estimation::Estimator::Instance()->update_gyro_scale()
	// estimation::Estimator::Instance()->update_accel_offset()
	// estimation::Estimator::Instance()->update_accel_scale()

	// estimation::Estimator::Instance()->update_mag_offset() ... we could get fancy and do elipsoid fitting.. only if neccessary
	// estimation::Estimator::Instance()->update_mag_scale()

	// estimation::Estimator::Instance()->reset() ... to reinitialize the estimator with the new offsets and scales
}