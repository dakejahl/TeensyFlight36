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
#include <DispatchQueue.hpp>
#include <Mpu9250.hpp>

void imu_task(void* args)
{
	auto mpu9250 = new Mpu9250();

	// Check to ensure device is alive
	bool alive = false;
	while (!alive)
	{
		// Just spin until the device is alive
		vTaskDelay(500);
		alive = mpu9250->probe();
	}

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

	for(;;)
	{
		// Only read sensor when there is new data available
		if (mpu9250->new_data_available())
		{
			mpu9250->collect_data();

			abs_time_t time = time::HighPrecisionTimer::Instance()->get_absolute_time_us();

			mpu9250->publish_accel_data(time);
			mpu9250->publish_gyro_data(time);

			// mpu9250->print_formatted_data();
		}
		else
		{
			early_counter++;
			SYS_INFO("mpu9250 data was not available: %u", early_counter);
		}

		vTaskDelay(1); // 1kHz, sometimes we miss some data buts that's OK... I think
	}
}