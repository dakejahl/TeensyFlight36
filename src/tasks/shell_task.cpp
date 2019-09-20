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
std::string GYRO_CAL = "cal gyro";
std::string ACCEL_CAL = "cal accel";
std::string MAG_CAL = "cal mag";


void evaluate_user_command(void);
void calibrate_gyro(void);
void calibrate_accel(void);

// Functions to allow streaming of data in CSV format
void stream_accel_data(void);
void stream_mag_data(void);
void stream_attitude_euler_data(void);
void stream_filtered_gyro_data(void);

// NOTE: used to send rate controller setpoints and rate actuals for controller tuning
void stream_controller_tuning(void);



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
		return;
	}
	else if (buffer == ACCEL_CAL)
	{
		SYS_INFO("Calibrating accel");
		calibrate_accel();
		return;
	}
	else if (buffer == MAG_CAL)
	{
		SYS_INFO("Calibrating mag");
		stream_mag_data();
		return;
	}
	else if (buffer == "stream accel")
	{
		SYS_INFO("Streaming accel data");
		stream_accel_data();
		return;
	}
	else if (buffer == "stream rpy")
	{
		SYS_INFO("Streaming euler rpy data");
		stream_attitude_euler_data();
		return;
	}
	else if (buffer == "stream gyrof")
	{
		SYS_INFO("Streaming filtered gyro data");
		stream_filtered_gyro_data();
		return;
	}
	else if (buffer == "stream controller tuning")
	{
		SYS_INFO("Streaming controller tuning data");
		stream_controller_tuning();
		return;
	}

	Serial.print("tsh> ");
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

void stream_mag_data(void)
{
	Serial4.begin(115200, SERIAL_8N1);

	messenger::Subscriber<mag_raw_data_s> mag_sub;

	SYS_INFO("Enabling mag data stream over serial4");

	for(;;)
	{
		if (mag_sub.updated())
		{
			auto data = mag_sub.get();
			float x = data.x;
			float y = data.y;
			float z = data.z;

			Serial4.print(x);
			Serial4.print(',');
			Serial4.print(y);
			Serial4.print(',');
			Serial4.print(z);
			Serial4.print("\n");
		}

		vTaskDelay(50);

		// Any user input cancels the spewing of data
		if (Serial.available())
		{
			SYS_INFO("Disabling mag data stream");
			return;
		}
	}
}

void stream_accel_data(void)
{
	Serial4.begin(115200, SERIAL_8N1);

	messenger::Subscriber<accel_raw_data_s> accel_sub;

	SYS_INFO("Enabling accel data stream over serial4");

	for(;;)
	{
		if (accel_sub.updated())
		{
			auto data = accel_sub.get();
			float x = data.x;
			float y = data.y;
			float z = data.z;

			Serial4.print(x);
			Serial4.print(',');
			Serial4.print(y);
			Serial4.print(',');
			Serial4.print(z);
			Serial4.print("\n");
		}

		vTaskDelay(50);

		// Any user input cancels the spewing of data
		if (Serial.available())
		{
			SYS_INFO("Disabling accel data stream");
			return;
		}
	}
}

void stream_attitude_euler_data(void)
{
	Serial4.begin(115200, SERIAL_8N1);

	messenger::Subscriber<attitude_euler_s> attitude_sub;

	SYS_INFO("Enabling euler attitude data stream over serial4");

	for(;;)
	{
		if (attitude_sub.updated())
		{
			auto data = attitude_sub.get();
			float x = data.roll * 180 / M_PI;
			float y = data.pitch * 180 / M_PI;
			float z = data.yaw * 180 / M_PI;

			Serial4.print(x);
			Serial4.print(',');
			Serial4.print(y);
			Serial4.print(',');
			Serial4.print(z);
			Serial4.print("\n");
		}

		// 20hz
		vTaskDelay(50);

		// Any user input cancels the spewing of data
		if (Serial.available())
		{
			SYS_INFO("Disabling euler attitude data stream");
			return;
		}
	}
}

void stream_filtered_gyro_data(void)
{
	Serial4.begin(115200, SERIAL_8N1);

	messenger::Subscriber<gyro_filtered_data_s> gyro_f_sub;

	SYS_INFO("Enabling filtered gyro data stream over serial4");

	for(;;)
	{
		if (gyro_f_sub.updated())
		{
			auto data = gyro_f_sub.get();
			float x = data.x;
			float y = data.y;
			float z = data.z;

			Serial4.print(x);
			Serial4.print(',');
			Serial4.print(y);
			Serial4.print(',');
			Serial4.print(z);
			Serial4.print("\n");

			// SYS_INFO("roll: %f", data.roll);
			// SYS_INFO("pitch: %f", data.pitch);
		}

		// 20hz
		vTaskDelay(50);

		// Any user input cancels the spewing of data
		if (Serial.available())
		{
			SYS_INFO("Disabling filtered gyro data stream");
			return;
		}
	}
}

void stream_controller_tuning(void)
{
	Serial4.begin(115200, SERIAL_8N1);

	messenger::Subscriber<rates_control_euler_s> rates_sub;
	messenger::Subscriber<setpoint_rates_s> rates_sp_sub;

	SYS_INFO("Enabling controller_tuning data stream over serial4");

	for(;;)
	{
		if (rates_sp_sub.updated())
		{
			auto rates_sp = rates_sp_sub.get();
			auto rates_act = rates_sub.get();

			float x = rates_sp.pitch;
			float y = rates_act.pitch;
			float z = 0;

			Serial4.print(x);
			Serial4.print(',');
			Serial4.print(y);
			Serial4.print(',');
			Serial4.print(z);
			Serial4.print("\n");

			// SYS_INFO("pitch: %f", y);
			// SYS_INFO("pitch sp: %f", x);
		}

		// 20hz
		vTaskDelay(50);

		// Any user input cancels the spewing of data
		if (Serial.available())
		{
			SYS_INFO("Disabling controller_tuning data stream");
			return;
		}
	}
}