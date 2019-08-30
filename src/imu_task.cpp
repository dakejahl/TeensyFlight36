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

	for(;;)
	{
		// Only read sensor when there is new data available
		if (mpu9250->new_data_available())
		{
			mpu9250->collect_sensor_data(&sensor_data);

			// int16_t accel_z = 0;
			// uint8_t msb = mpu9250->read_register(address::ACCEL_ZOUT_H);
			// uint8_t lsb = mpu9250->read_register(address::ACCEL_ZOUT_L);

			// uint8_t* ptr = static_cast<uint8_t*>((void*)&accel_z);
			// ptr[0] = msb;
			// ptr[1] = lsb;

			// Convert data to real world values
			// TODO: calibration
			#define TEMP_CALIB_OFFSET 0
			// float temperature = (sensor_data.temp - TEMP_CALIB_OFFSET) / 333.87f + 21.0f;
			// SYS_INFO("temperature_raw: %d", sensor_data.temp);

			#define ACCEL_CALIB_OFFSET 0
			#define ACCEL_CALIB_SCALE 1

			static constexpr float CONSTANTS_ONE_G = 9.80665f; // m/s^2
			static constexpr float TICK_PER_G = 2048;
			static constexpr float ACCEL_SCALE_FACTOR = CONSTANTS_ONE_G / TICK_PER_G;

			// value (m/s^2) = (sensor_data.gyro_x / TICK_PER_G) * CONSTANTS_ONE_G
			float accel_z = ((sensor_data.accel_z * ACCEL_SCALE_FACTOR) - ACCEL_CALIB_OFFSET) * ACCEL_CALIB_SCALE;
			SYS_INFO("accel_z_raw: %f", accel_z);
		}
		else
		{
			early_counter++;
			SYS_INFO("mpu9250 data was not available: %u", early_counter);
		}

		vTaskDelay(50);
	}
}