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
#include <GyroCalibration.hpp>
#include <AccelCalibration.hpp>


std::string buffer;
std::string GYRO_CAL = "gyro cal";
std::string ACCEL_CAL = "accel cal";
std::string MAG_CAL = "mag cal";


void evaluate_user_command(void);
void calibrate_gyro(void);
void calibrate_accel(void);
void calibrate_mag(void);


// This task will poll the USB interface for user commands:
// - gyro calibration, let sit still and measure offsets on all 3 axis (and noise value for ekf?)
// - accel calibration (6 sides), find offsets and scales for all 3 axis
// - mag calibration (rotate A LOT, find max/min on all 3 axis, calculate offsets and scale factor)
void shell_task(void* args)
{
	char byte = 0;

	for(;;)
	{
		// NOTE: Serial port PC side sends strings at a time terminated with a LF
		if (Serial.available())
		{
			byte = Serial.read();

			if (byte == 10) // line feed
			{
				evaluate_user_command();
				buffer.clear();
			}
			else
			{
				buffer.push_back(byte);
			}
		}
		else
		{
			vTaskDelay(100); // run at 10hz
		}
	}
}

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
	else if (buffer == MAG_CAL)
	{
		SYS_INFO("Calibrating mag");
		calibrate_mag();
	}
}

void calibrate_gyro(void)
{
	GyroCalibration gyro;

	gyro.calibrate();
}

// For this one we want to put the sensor on each side (6 sides) and measure the static reading.
// The static reading should just be the gravity field vector "g" which we know is 9.81 m/s2.
// We will just look for the value on an axis to be greater than 9, at which point we will
// just delay 2 seconds, and then sample for 3 seconds at 10hz (30 samples)
void calibrate_accel(void)
{
	AccelCalibration accel;

	while (!accel.all_sides_complete())
	{
		auto side = accel.get_next_side_to_calibrate();

		accel.calibrate(side);
	}

	// The gravity vector has been measured on each side, we now want to find the offsets and scales.
	accel.calculate_offsets_and_scales();

	// TODO:
	// estimation::Estimator::Instance()->update_gyro_offset() ... or something like that
	// estimation::Estimator::Instance()->update_gyro_scale()
	// estimation::Estimator::Instance()->update_accel_offset()
	// estimation::Estimator::Instance()->update_accel_scale()

	// estimation::Estimator::Instance()->update_mag_offset() ... we could get fancy and do elipsoid fitting.. only if neccessary
	// estimation::Estimator::Instance()->update_mag_scale()

	// estimation::Estimator::Instance()->reset() ... to reinitialize the estimator with the new offsets and scales
}

void calibrate_mag(void)
{

	// NO

	// we are just going to do the ellipsoid fit on the host PC -- so much easier that way





	// number of points to collect
	// MagCalibration mag;

	// mag.collect_points(10); // 10ms interval

	// SYS_INFO("collected all points");

	// mag.apply_ellipsoid_fit(); // 10ms interval

	// SYS_INFO("performed fit");

	// auto x_c = mag.get_x_center();
	// auto y_c = mag.get_y_center();
	// auto z_c = mag.get_z_center();

	// auto x_r = mag.get_x_radius();
	// auto y_r = mag.get_y_radius();
	// auto z_r = mag.get_z_radius();

	// // LETS SEE HOW GOOD OUR SHIT IS!
	// messenger::Subscriber<mag_raw_data_s> mag_sub;

	// for(;;)
	// {
	// 	auto data = mag_sub.get();

	// 	// apply corrections
	// 	float x = (data.x - x_c) / x_r;
	// 	float y = (data.y - y_c) / y_r;
	// 	float z = (data.z - z_c) / z_r;


	// 	Serial4.print(x);
	// 	Serial4.print(',');
	// 	Serial4.print(y);
	// 	Serial4.print(',');
	// 	Serial4.print(z);
	// 	Serial4.print("\n");


	// 	vTaskDelay(10);
	// }
}
