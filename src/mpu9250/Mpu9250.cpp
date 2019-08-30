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
	write_register(address::CONFIG, value::CONFIG_NO_DLPF_GYRO_3600Hz);

	// Sample rate divider
	write_register(address::SMPLRT_DIV, value::SMPLRT_DIV_NONE);

	// User control
	write_register(address::USER_CTRL, value::BIT_I2C_IF_DIS | value::BIT_SIG_COND_RST);

	// Interrupt enable
	write_register(address::INT_ENABLE, value::INT_DISABLE);

	// Sensors
	write_register(address::GYRO_CONFIG, value::GYRO_NO_DLPF_2000_DPS);
	write_register(address::ACCEL_CONFIG, value::ACCEL_16_G);
	write_register(address::ACCEL_CONFIG_2, value::ACCEL_NO_DLPF_4kHz);
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
	if (reg_val != value::CONFIG_NO_DLPF_GYRO_3600Hz)
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
	if (reg_val != value::GYRO_NO_DLPF_2000_DPS)
		return false;

	reg_val = read_register(address::ACCEL_CONFIG);
	if (reg_val != value::ACCEL_16_G)
		return false;

	reg_val = read_register(address::ACCEL_CONFIG_2);
	if (reg_val != value::ACCEL_NO_DLPF_4kHz)
		return false;

	return true;
}