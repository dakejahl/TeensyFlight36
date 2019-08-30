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

#include <Mpu9250.hpp>

void imu_task(void* args)
{
	auto dispatcher = new DispatchQueue("dummy_q");
	// vTaskDelay(1000);

	auto func1 = []
	{
		volatile unsigned dummy = 0;
		for (unsigned i = 0; i < 10; ++i)
		{
			dummy++;
		}

		// SYS_INFO("Hey I got dispatched on an interval!");
	};

	dispatcher->dispatch_on_interval(func1, 1000);

	auto mpu9250 = new Mpu9250();

	// Check to ensure device is alive
	bool alive = mpu9250->probe();

	// Configure register settings
	mpu9250->initialize_registers();

	// Check that the configuration is correct
	bool registers_okay = mpu9250->validate_registers();

	if (alive && registers_okay)
	{
		SYS_INFO("mpu9250 is ALIVE and registers are set!");
	}
	else if (alive)
	{
		SYS_INFO("... alive but registers fucked up ...");
	}
	else
	{
		SYS_INFO("... not alive ...");
	}

	static unsigned early_counter = 0;

	struct __attribute__((__packed__))
	{
		int16_t		accel_x;
		int16_t		accel_y;
		int16_t		accel_z;
		int16_t		temp;
		int16_t		gyro_x;
		int16_t		gyro_y;
		int16_t		gyro_z;
		int16_t		padding;
	} sensor_data;

	// Set up our publisher
	messenger::Publisher<accel_raw_data_s> accel_pub;
	messenger::Publisher<gyro_raw_data_s> gyro_pub;

	for(;;)
	{
		// Only read sensor when there is new data available
		if (mpu9250->new_data_available())
		{
			mpu9250->collect_sensor_data(&sensor_data);

			// TODO: calibration
			#define TEMP_CALIB_OFFSET 0
			#define ACCEL_CALIB_OFFSET 0
			#define ACCEL_CALIB_SCALE 1
			#define GYRO_CALIB_OFFSET 0
			#define GYRO_CALIB_SCALE 1

			// Accel conversion
			static constexpr double CONSTANTS_ONE_G = 9.80665; // m/s^2
			static constexpr double TICK_PER_G = 2048.0;
			static constexpr float ACCEL_M_S2_PER_TICK = CONSTANTS_ONE_G / TICK_PER_G;

			// Gyro conversion
			static constexpr double GYRO_FULL_SCALE_DPS = 2000.0;
			static constexpr double GYRO_FULL_SCALE_RAD_S = 2000.0 / (180.0 / M_PI);
			static constexpr unsigned TICKS = 65535;
			static constexpr float RAD_S_PER_TICK = 2 * GYRO_FULL_SCALE_RAD_S / TICKS; // times 2 since +/- FS

			// Apply accel conversion
			float accel_x = ((sensor_data.accel_x * ACCEL_M_S2_PER_TICK) - ACCEL_CALIB_OFFSET) * ACCEL_CALIB_SCALE;
			float accel_y = ((sensor_data.accel_y * ACCEL_M_S2_PER_TICK) - ACCEL_CALIB_OFFSET) * ACCEL_CALIB_SCALE;
			float accel_z = ((sensor_data.accel_z * ACCEL_M_S2_PER_TICK) - ACCEL_CALIB_OFFSET) * ACCEL_CALIB_SCALE;
			// SYS_INFO("raw_accel_z: %d", sensor_data.accel_z);
			SYS_INFO("accel_z: %f", accel_z);

			// Apply gyro conversion
			float gyro_x = ((sensor_data.gyro_x * RAD_S_PER_TICK) - GYRO_CALIB_OFFSET) * GYRO_CALIB_SCALE;
			float gyro_y = ((sensor_data.gyro_y * RAD_S_PER_TICK) - GYRO_CALIB_OFFSET) * GYRO_CALIB_SCALE;
			float gyro_z = ((sensor_data.gyro_z * RAD_S_PER_TICK) - GYRO_CALIB_OFFSET) * GYRO_CALIB_SCALE;
			// SYS_INFO("raw_gyro_x: %d", sensor_data.gyro_x);
			SYS_INFO("gyro_x: %f", gyro_x);

			// Apply temp conversion -- see datasheet
			float temperature = (sensor_data.temp - TEMP_CALIB_OFFSET) / 333.87f + 21.0f;
			// SYS_INFO("raw_temperature: %d", sensor_data.temp);
			SYS_INFO("temperature: %f", temperature);
			SYS_INFO("--- --- --- --- --- --- ---");
		}
		else
		{
			early_counter++;
			SYS_INFO("mpu9250 data was not available: %u", early_counter);
		}

		// WARNING: since we sample at 1kHz, we must not check the registers at 1kHz otherwise we will be too early
		// NOTE: Spamming read requests when data is not ready fucks with the sensor and causes a long data blackout (~20ms?)
		vTaskDelay(100); // 500Hz seems solid for now
	}
}