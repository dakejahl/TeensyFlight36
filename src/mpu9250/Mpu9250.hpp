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
#include <LowPassFilter.hpp>


// Gyro constants
static constexpr double GYRO_FULL_SCALE_DPS = 2000.0;
static constexpr double GYRO_FULL_SCALE_RAD_S = GYRO_FULL_SCALE_DPS / (180.0 / M_PI);
static constexpr unsigned TICKS = 65536;
static constexpr float RAD_S_PER_TICK = 2 * GYRO_FULL_SCALE_RAD_S / TICKS; // times 2 since +/- FS
static constexpr float DEG_S_PER_TICK = RAD_S_PER_TICK * 180 / M_PI; // times 2 since +/- FS


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
		// Mag data is stored in the first slot for external sensors if enabled
		uint8_t 	mag_st1;
		int16_t 	mag_x;
		int16_t 	mag_y;
		int16_t 	mag_z;
		uint8_t 	mag_st2;
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

	bool new_data_available(void);
	void collect_data(void);

	void publish_accel_data(abs_time_t& timestamp);
	void publish_gyro_data(abs_time_t& timestamp);
	void publish_mag_data(abs_time_t& timestamp);

	void print_formatted_data(void);

	// calibration setters
	void set_gyro_calibration(float offset, float scale)
	{
		_gyro_offset = offset;
		_gyro_scale = scale;
	}

	void set_accel_calibration(float offset, float scale)
	{
		_accel_offset = offset;
		_accel_scale = scale;
	}

	void set_mag_calibration(float offset, float scale)
	{
		_mag_offset = offset;
		_mag_scale = scale;
	}

private:
	void initialize_magnetometer_registers(void);

	// Read / Write for the MPU9250
	void write_register(uint8_t addr, uint8_t val);
	uint8_t read_register(uint8_t reg);

	// Read / Write for the AK8963
	void write_register_mag(uint8_t addr, uint8_t val);
	uint8_t read_register_mag(uint8_t reg);

	void get_mag_factory_cal(void);


	interface::Spi* _interface;

	Mpu9250::SensorDataPacked _sensor_data;

	messenger::Publisher<accel_raw_data_s> _accel_pub;
	messenger::Publisher<gyro_raw_data_s> _gyro_pub;
	messenger::Publisher<mag_raw_data_s> _mag_pub;
	messenger::Publisher<gyro_filtered_data_s> _filtered_gyro_pub;


	// mag factory cal "sensitivity adjustment"
	float _mag_factory_scale_factor_x = 0;
	float _mag_factory_scale_factor_y = 0;
	float _mag_factory_scale_factor_z = 0;

	// User calibration
	float _gyro_offset = 0;
	float _gyro_scale= 0;

	float _accel_offset = 0;
	float _accel_scale = 0;

	float _mag_offset = 0;
	float _mag_scale = 0;

	abs_time_t _last_timestamp = 0;

	// mag filter
	LowPassFilter<float> _mag_filter_x {20}; // 20Hz filter
	LowPassFilter<float> _mag_filter_y {20}; // 20Hz filter
	LowPassFilter<float> _mag_filter_z {20}; // 20Hz filter

	LowPassFilter<float> _gyro_filter_x {30};// 30Hz filter
	LowPassFilter<float> _gyro_filter_y {30};// 30Hz filter
	LowPassFilter<float> _gyro_filter_z {30};// 30Hz filter

	LowPassFilter<float> _accel_filter_x {20};// 50Hz filter
	LowPassFilter<float> _accel_filter_y {20};// 50Hz filter
	LowPassFilter<float> _accel_filter_z {20};// 50Hz filter
};

