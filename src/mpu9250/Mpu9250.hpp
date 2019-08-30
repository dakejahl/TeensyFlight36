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

#include <Spi.hpp>
#include <mpu9250_registers.hpp>

namespace mpu9250_spi
{

static constexpr uint8_t CS = 10;
static constexpr uint8_t BUS = 0;
static constexpr unsigned FREQ = 8000000;

} // end namespace mpu9250_spi

class Mpu9250
{
public:

	Mpu9250()
	{
		// Initialize the SPI interface
		_interface = new interface::Spi(mpu9250_spi::BUS, mpu9250_spi::FREQ, mpu9250_spi::CS);
	}

	void initialize_registers(void);

	bool validate_registers(void);

	bool probe(void);

	uint8_t read_register(uint8_t reg);
	void write_register(uint8_t addr, uint8_t val);

	void collect_sensor_data(void* data);

	bool new_data_available(void);

private:

	interface::Spi* _interface;

};