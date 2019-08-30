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

#include <mpu9250_registers.hpp>

#include <Spi.hpp>
#include <Messenger.hpp>

// TODO: calibration
#define TEMP_CALIB_OFFSET 0
#define ACCEL_CALIB_OFFSET 0
#define ACCEL_CALIB_SCALE 1
#define GYRO_CALIB_OFFSET 0
#define GYRO_CALIB_SCALE 1

// Gyro constants
static constexpr double GYRO_FULL_SCALE_DPS = 2000.0;
static constexpr double GYRO_FULL_SCALE_RAD_S = GYRO_FULL_SCALE_DPS / (180.0 / M_PI);
static constexpr unsigned TICKS = 65536;
static constexpr float RAD_S_PER_TICK = 2 * GYRO_FULL_SCALE_RAD_S / TICKS; // times 2 since +/- FS

// Accel constants
static constexpr double CONSTANTS_ONE_G = 9.80665; // m/s^2
static constexpr double TICK_PER_G = 2048.0; // 65536 / 2
static constexpr float ACCEL_M_S2_PER_TICK = CONSTANTS_ONE_G / TICK_PER_G;

namespace mpu9250_spi
{

static constexpr uint8_t CS = 10;
static constexpr uint8_t BUS = 0;
static constexpr unsigned FREQ = 8000000;

} // end namespace mpu9250_spi

class Mpu9250
{
	struct __attribute__((__packed__)) SensorDataPacked
	{
		int16_t		accel_x;
		int16_t		accel_y;
		int16_t		accel_z;
		int16_t		temperature;
		int16_t		gyro_x;
		int16_t		gyro_y;
		int16_t		gyro_z;
		int16_t		padding;
	};

public:

	Mpu9250()
	{
		// Initialize the SPI interface
		_interface = new interface::Spi(mpu9250_spi::BUS, mpu9250_spi::FREQ, mpu9250_spi::CS);
	}

	bool probe(void);
	void initialize_registers(void);
	bool validate_registers(void);

	uint8_t read_register(uint8_t reg);
	void write_register(uint8_t addr, uint8_t val);

	bool new_data_available(void);
	void collect_sensor_data(void);

	void publish_accel_data(abs_time_t& timestamp);
	void publish_gyro_data(abs_time_t& timestamp);

	void print_formatted_data(void);

private:

	interface::Spi* _interface;

	Mpu9250::SensorDataPacked _sensor_data;

	messenger::Publisher<accel_raw_data_s> _accel_pub;
	messenger::Publisher<gyro_raw_data_s> _gyro_pub;
};