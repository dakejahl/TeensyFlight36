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
#include <Mpu9250.hpp>

bool Mpu9250::probe(void)
{
	uint8_t whoami = read_register(address::WHOAMI);

	// The whoami is 0x71
	if (whoami == value::WHOAMI)
	{
		return true;
	}

	return false;
}

uint8_t Mpu9250::read_register(uint8_t reg)
{
	uint8_t send_buf = reg | 1<<7;
	uint8_t recv_buf = 0;

	_interface->transfer(&send_buf, 1, &recv_buf, 1);

	return recv_buf;
}

void Mpu9250::write_register(uint8_t addr, uint8_t val)
{
	uint8_t send_buf[2] = {addr, val};

	_interface->transfer(send_buf, 2, nullptr, 0);
}

void Mpu9250::initialize_registers(void)
{
	// Power management
	write_register(address::PWR_MGMT_1, value::CLK_SEL_AUTO);
	write_register(address::PWR_MGMT_2, value::ENABLE_ACCEL_GYRO);

	// Configuration
	write_register(address::CONFIG, value::CONFIG_DLPF_GYRO_92Hz);

	// Sample rate divider
	write_register(address::SMPLRT_DIV, value::SMPLRT_DIV_NONE);

	// User control
	write_register(address::USER_CTRL, value::BIT_I2C_IF_DIS | value::BIT_SIG_COND_RST);

	// Interrupt enable
	write_register(address::INT_ENABLE, value::INT_DISABLE);

	// Sensors
	write_register(address::GYRO_CONFIG, value::GYRO_DLPF_2000_DPS);
	write_register(address::ACCEL_CONFIG, value::ACCEL_16_G);
	write_register(address::ACCEL_CONFIG_2, value::ACCEL_DLPF_1kHz);
}

bool Mpu9250::validate_registers(void)
{
	uint8_t reg_val = 0;

	// Power management
	reg_val = read_register(address::PWR_MGMT_1);
	if (reg_val != value::CLK_SEL_AUTO)
		return false;

	reg_val = read_register(address::PWR_MGMT_2);
	if (reg_val != value::ENABLE_ACCEL_GYRO)
		return false;

	// Configuration
	reg_val = read_register(address::CONFIG);
	if (reg_val != value::CONFIG_DLPF_GYRO_92Hz)
		return false;

	// Sample rate divider
	reg_val = read_register(address::SMPLRT_DIV);
	if (reg_val != value::SMPLRT_DIV_NONE)
		return false;

	// Interrupt enable
	reg_val = read_register(address::INT_ENABLE);
	if (reg_val != value::INT_DISABLE)
		return false;

	// Sensors
	reg_val = read_register(address::GYRO_CONFIG);
	if (reg_val != value::GYRO_DLPF_2000_DPS)
		return false;

	reg_val = read_register(address::ACCEL_CONFIG);
	if (reg_val != value::ACCEL_16_G)
		return false;

	reg_val = read_register(address::ACCEL_CONFIG_2);
	if (reg_val != value::ACCEL_DLPF_1kHz)
		return false;

	return true;
}

bool Mpu9250::new_data_available(void)
{
	return read_register(address::INT_STATUS) == value::RAW_DATA_RDY_INT;
}

void Mpu9250::collect_data(void)
{
	uint8_t* byte_data = reinterpret_cast<uint8_t*>(&_sensor_data);

	// Accel (xyz)   temp(c)   Gyro(xyz)
	static constexpr size_t num_axis = 7;
	static constexpr size_t bytes_per_axis = 2;
	static constexpr size_t bytes_to_read = num_axis + bytes_per_axis;

	for (size_t i = 0; i < bytes_to_read; i++)
	{
		// Start at base address and increment over all registers we are interested in.
		uint8_t val = read_register(address::ACCEL_XOUT_H + i);

		// We need to reorder the LSB and MSB for each axis pair
		bool at_new_half_word = !(i % 2);

		if (at_new_half_word)
		{
			byte_data[i + 1] = val;
		}
		else
		{
			byte_data[i - 1] = val;
		}
	}

}

void Mpu9250::publish_accel_data(abs_time_t& timestamp)
{
	// Convert the raw data
	float x = ((_sensor_data.accel_x * ACCEL_M_S2_PER_TICK) - ACCEL_CALIB_OFFSET) * ACCEL_CALIB_SCALE;
	float y = ((_sensor_data.accel_y * ACCEL_M_S2_PER_TICK) - ACCEL_CALIB_OFFSET) * ACCEL_CALIB_SCALE;
	float z = ((_sensor_data.accel_z * ACCEL_M_S2_PER_TICK) - ACCEL_CALIB_OFFSET) * ACCEL_CALIB_SCALE;
	float temp = (_sensor_data.temperature - TEMP_CALIB_OFFSET) / 333.87f + 21.0f;

	// Stuff the message
	accel_raw_data_s data;

	data.timestamp = timestamp;
	data.x = x;
	data.y = y;
	data.z = z;
	data.temperature = temp;

	_accel_pub.publish(data);
}

void Mpu9250::publish_gyro_data(abs_time_t& timestamp)
{
	// Convert the raw data
	float x = ((_sensor_data.gyro_x * RAD_S_PER_TICK) - GYRO_CALIB_OFFSET) * GYRO_CALIB_SCALE;
	float y = ((_sensor_data.gyro_y * RAD_S_PER_TICK) - GYRO_CALIB_OFFSET) * GYRO_CALIB_SCALE;
	float z = ((_sensor_data.gyro_z * RAD_S_PER_TICK) - GYRO_CALIB_OFFSET) * GYRO_CALIB_SCALE;
	float temp = (_sensor_data.temperature - TEMP_CALIB_OFFSET) / 333.87f + 21.0f;

	// Stuff the message
	gyro_raw_data_s data;

	data.timestamp = timestamp;
	data.x = x;
	data.y = y;
	data.z = z;
	data.temperature = temp;

	_gyro_pub.publish(data);
}

void Mpu9250::print_formatted_data(void)
{
	float accel_z = ((_sensor_data.accel_z * ACCEL_M_S2_PER_TICK) - ACCEL_CALIB_OFFSET) * ACCEL_CALIB_SCALE;
	float gyro_x = ((_sensor_data.gyro_x * RAD_S_PER_TICK) - GYRO_CALIB_OFFSET) * GYRO_CALIB_SCALE;
	float temperature = (_sensor_data.temperature - TEMP_CALIB_OFFSET) / 333.87f + 21.0f;

	SYS_INFO("accel_z: %f", accel_z);
	SYS_INFO("gyro_x: %f", gyro_x);
	SYS_INFO("temperature: %f", temperature);
	SYS_INFO("--- --- --- --- --- --- ---");
}
