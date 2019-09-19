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

	// Configure the mag
	initialize_magnetometer_registers();
}

bool Mpu9250::validate_registers(void)
{
	uint8_t reg_val = 0;

	// Power management
	reg_val = read_register(address::PWR_MGMT_1);
	if (reg_val != value::CLK_SEL_AUTO)
	{
		SYS_INFO("PWR_MGMT_1");
		return false;
	}

	reg_val = read_register(address::PWR_MGMT_2);
	if (reg_val != value::ENABLE_ACCEL_GYRO)
	{
		SYS_INFO("PWR_MGMT_2");
		return false;
	}

	// Configuration
	reg_val = read_register(address::CONFIG);
	if (reg_val != value::CONFIG_DLPF_GYRO_92Hz)
	{
		SYS_INFO("CONFIG");
		return false;
	}

	// Sample rate divider
	reg_val = read_register(address::SMPLRT_DIV);
	if (reg_val != value::SMPLRT_DIV_NONE)
	{
		SYS_INFO("SMPLRT_DIV");
		return false;
	}

	// Interrupt enable
	reg_val = read_register(address::INT_ENABLE);
	if (reg_val != value::INT_DISABLE)
	{
		SYS_INFO("INT_ENABLE");
		return false;
	}

	// Sensors
	reg_val = read_register(address::GYRO_CONFIG);
	if (reg_val != value::GYRO_DLPF_2000_DPS)
	{
		SYS_INFO("GYRO_CONFIG");
		return false;
	}

	reg_val = read_register(address::ACCEL_CONFIG);
	if (reg_val != value::ACCEL_16_G)
	{
		SYS_INFO("ACCEL_CONFIG");
		return false;
	}

	reg_val = read_register(address::ACCEL_CONFIG_2);
	if (reg_val != value::ACCEL_DLPF_1kHz)
	{
		SYS_INFO("ACCEL_CONFIG_2");
		return false;
	}

	return true;
}

uint8_t Mpu9250::read_register_mag(uint8_t reg)
{
	write_register(address::I2C_SLV0_ADDR, value::AK8963_I2C_ADDR | value::BIT_I2C_SLV0_READ); // setup to read from i2c slave
	write_register(address::I2C_SLV0_REG, reg); // I2C slave 0 register address to read from
	write_register(address::I2C_SLV0_CTRL, value::BIT_I2C_SLV0_EN | 1); // bytes read [3:0] == 7 bytes max

	// 25 μs/byte at 400 kHz
	vTaskDelay(10);

	auto byte = read_register(address::EXT_SENS_DATA_00); // collect value

	write_register(address::I2C_SLV0_CTRL, 0); // disable slave writing to ext sensor registers

	vTaskDelay(10);

	return byte;
}

void Mpu9250::write_register_mag(uint8_t addr, uint8_t val)
{
	write_register(address::I2C_SLV0_ADDR, value::AK8963_I2C_ADDR | value::BIT_I2C_SLV0_WRITE);
	write_register(address::I2C_SLV0_REG, addr);
	write_register(address::I2C_SLV0_DO, val);
	write_register(address::I2C_SLV0_CTRL, value::BIT_I2C_SLV0_EN | 1);
	vTaskDelay(50);
}

void Mpu9250::initialize_magnetometer_registers(void)
{
	// User control -- enable i2c master and reset everything
	auto reg = read_register(address::USER_CTRL);
	write_register(address::USER_CTRL, reg | value::I2C_MST_EN | value::BIT_SIG_COND_RST);

	// I2C master
	write_register(address::I2C_MSTR_CTRL, value::BIT_I2C_MST_P_NSR | value::BIT_I2C_MST_WAIT_FOR_ES | value::BIT_I2C_MST_CLOCK_400kHz);

	// Check to ensure mag is alive
	auto whoami = read_register_mag(address::AK8963_WHOAMI);
	if (whoami != value::AK8963_DEVICE_ID)
	{
		SYS_INFO("Magnetometer is not alive: %d", whoami);
	}
	else
	{
		SYS_INFO("Magnetometer is alive!");
	}

	get_mag_factory_cal();

	// Continuous measurement at highest resolution
	uint8_t mode = value::AK8963_CONTINUOUS_MODE2 | value::AK8963_16BIT_ADC;

	write_register(address::I2C_SLV0_ADDR, value::AK8963_I2C_ADDR | value::BIT_I2C_SLV0_WRITE);
	write_register(address::I2C_SLV0_REG, address::AK8963_CNTL1);
	write_register(address::I2C_SLV0_DO, mode);
	write_register(address::I2C_SLV0_CTRL, value::BIT_I2C_SLV0_EN | 1);
	vTaskDelay(50);

	// Now we read the cntl1 register to verify settings
	write_register(address::I2C_SLV0_ADDR, value::AK8963_I2C_ADDR | value::BIT_I2C_SLV0_READ);
	write_register(address::I2C_SLV0_REG, address::AK8963_CNTL1);
	write_register(address::I2C_SLV0_CTRL, value::BIT_I2C_SLV0_EN | 1);
	vTaskDelay(100);

	if(read_register(address::EXT_SENS_DATA_00) != mode)
	{
		SYS_INFO("Magnetometer is not configured!");
	}
	else
	{
		SYS_INFO("Magnetometer is configured!");
	}

	// Setup to dump mag sensor data into external sensor0 registers
	write_register(address::I2C_SLV0_ADDR, value::AK8963_I2C_ADDR | value::BIT_I2C_SLV0_READ);
	write_register(address::I2C_SLV0_REG, 0x02); // st1
	write_register(address::I2C_SLV0_CTRL, value::BIT_I2C_SLV0_EN | 8); // read st1 - st2
	vTaskDelay(50);
}

void Mpu9250::get_mag_factory_cal(void)
{
	// Extra factory calibration values
	// -- first, reset
	write_register_mag(address::AK8963_CNTL2, value::AK8963_RESET);
	// -- second, power down mag
	write_register_mag(address::AK8963_CNTL1, 0x00);
	// -- third, enter fuse ROM mode
	write_register_mag(address::AK8963_CNTL1, value::AK8963_FUSE_ROM);
	// -- fourth, read factory calibration from ROM
	write_register(address::I2C_SLV0_ADDR, value::AK8963_I2C_ADDR | value::BIT_I2C_SLV0_READ);
	write_register(address::I2C_SLV0_REG, address::AK8963_ASAX);
	write_register(address::I2C_SLV0_CTRL, value::BIT_I2C_SLV0_EN | 3); // read out all 3 calibration bytes
	vTaskDelay(50);
	// go collect the bytes I've just told the mpu9250 to read
	auto x = read_register(address::EXT_SENS_DATA_00);
	auto y = read_register(address::EXT_SENS_DATA_01);
	auto z = read_register(address::EXT_SENS_DATA_02);

	SYS_INFO("mag_cal_x is %d!", x);
	SYS_INFO("mag_cal_y is %d!", y);
	SYS_INFO("mag_cal_z is %d!", z);

	_mag_factory_scale_factor_x = (x - 128) * 0.5 / 128 + 1;
	_mag_factory_scale_factor_y = (y - 128) * 0.5 / 128 + 1;
	_mag_factory_scale_factor_z = (z - 128) * 0.5 / 128 + 1;
}

bool Mpu9250::new_data_available(void)
{
	return read_register(address::INT_STATUS) == value::RAW_DATA_RDY_INT;
}

void Mpu9250::collect_data(void)
{
	uint8_t* byte_data = reinterpret_cast<uint8_t*>(&_sensor_data);

	// Accel (xyz)   temp(c)   Gyro(xyz)   Mag (st1, xyz, st2)
	static constexpr size_t bytes_to_read = sizeof(SensorDataPacked) - 2; // 2 bytes of padding

	for (size_t i = 0; i < bytes_to_read; i++)
	{
		// Start at base address and increment over all registers we are interested in.
		byte_data[i] = read_register(address::ACCEL_XOUT_H + i);
	}

	// reorder the LSB and MSB in the sensor data
	SensorDataPacked temporaray = _sensor_data;

	uint8_t* copy = static_cast<uint8_t*>((void*)&temporaray);
	uint8_t* data = static_cast<uint8_t*>((void*)&_sensor_data);

	// accel xyz
	data[0] = copy[1];
	data[1] = copy[0];
	data[2] = copy[3];
	data[3] = copy[2];
	data[4] = copy[5];
	data[5] = copy[4];
	// temperature
	data[6] = copy[7];
	data[7] = copy[6];
	// gyro xyz
	data[8] = copy[9];
	data[9] = copy[8];
	data[10] = copy[11];
	data[11] = copy[10];
	data[12] = copy[13];
	data[13] = copy[12];
	// Mag data is already correct
}

void Mpu9250::publish_accel_data(abs_time_t& timestamp)
{
	// Convert the raw data
	float x = _sensor_data.accel_x * ACCEL_M_S2_PER_TICK;
	float y = _sensor_data.accel_y * ACCEL_M_S2_PER_TICK;
	float z = _sensor_data.accel_z * ACCEL_M_S2_PER_TICK;
	float temp = _sensor_data.temperature / 333.87f + 21.0f;

	// Apply a lowpass filter
	x = _accel_filter_x.apply(x, timestamp);
	y = _accel_filter_y.apply(y, timestamp);
	z = _accel_filter_z.apply(z, timestamp);

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
	float x = _sensor_data.gyro_x * DEG_S_PER_TICK;
	float y = _sensor_data.gyro_y * DEG_S_PER_TICK;
	float z = _sensor_data.gyro_z * DEG_S_PER_TICK;
	float temp = _sensor_data.temperature  / 333.87f + 21.0f;

	// Stuff the message
	gyro_raw_data_s data;

	data.timestamp = timestamp;
	data.x = x;
	data.y = y;
	data.z = z;
	data.temperature = temp;

	_gyro_pub.publish(data);

	// TODO: is there a better way to do this?
	// publish filtered data for controller input
	gyro_filtered_data_s filtered_gyro;

	x = _gyro_filter_x.apply(x, timestamp);
	y = _gyro_filter_y.apply(y, timestamp);
	z = _gyro_filter_z.apply(z, timestamp);

	filtered_gyro.timestamp = timestamp;
	filtered_gyro.x = x;
	filtered_gyro.y = y;
	filtered_gyro.z = z;
	filtered_gyro.temperature = temp;

	_filtered_gyro_pub.publish(filtered_gyro);
}

void Mpu9250::publish_mag_data(abs_time_t& timestamp)
{
	// Convert the raw data
	float x = _sensor_data.mag_x * _mag_factory_scale_factor_x;
	float y = _sensor_data.mag_y * _mag_factory_scale_factor_y;
	float z = _sensor_data.mag_z * _mag_factory_scale_factor_z;
	float temp = _sensor_data.temperature  / 333.87f + 21.0f;

	// Pass through a 50Hz LPF
	x = _mag_filter_x.apply(x, timestamp);
	y = _mag_filter_y.apply(y, timestamp);
	z = _mag_filter_z.apply(z, timestamp);

	// Stuff the message
	mag_raw_data_s data;

	data.timestamp = timestamp;
	data.x = x;
	data.y = y;
	data.z = z;
	data.temperature = temp;

	// TODO: collect timestamp inside of collect()
	_last_timestamp = timestamp;

	_mag_pub.publish(data);
}

void Mpu9250::print_formatted_data(void)
{
	float accel_x = _sensor_data.accel_x * ACCEL_M_S2_PER_TICK;
	float accel_y = _sensor_data.accel_y * ACCEL_M_S2_PER_TICK;
	float accel_z = _sensor_data.accel_z * ACCEL_M_S2_PER_TICK;

	float gyro_x = _sensor_data.gyro_x * DEG_S_PER_TICK;
	float gyro_y = _sensor_data.gyro_y * DEG_S_PER_TICK;
	float gyro_z = _sensor_data.gyro_z * DEG_S_PER_TICK;

	float temperature = _sensor_data.temperature / 333.87f + 21.0f;

	float mag_st1 = _sensor_data.mag_st1;
	float mag_x = _sensor_data.mag_x;
	float mag_y = _sensor_data.mag_y;
	float mag_z = _sensor_data.mag_z;
	float mag_st2 = _sensor_data.mag_st2;

	SYS_INFO("accel_x: %f", accel_x);
	SYS_INFO("accel_y: %f", accel_y);
	SYS_INFO("accel_z: %f", accel_z);
	SYS_INFO("gyro_x: %f", gyro_x);
	SYS_INFO("gyro_y: %f", gyro_y);
	SYS_INFO("gyro_z: %f", gyro_z);
	SYS_INFO("temperature: %f", temperature);
	SYS_INFO("mag_st1: %f", mag_st1);
	SYS_INFO("mag_x: %f", mag_x);
	SYS_INFO("mag_y: %f", mag_y);
	SYS_INFO("mag_z: %f", mag_z);
	SYS_INFO("mag_st2: %f", mag_st2);


	SYS_INFO("--- --- --- --- --- --- ---");
}
