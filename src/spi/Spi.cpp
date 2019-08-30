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

#include <Spi.hpp>

namespace interface
{

Spi::Spi(uint8_t bus, unsigned freq, uint8_t cs)
	: _chip_select(cs)
{
	// Compare the bus number with bitmask
	uint8_t bus_mask = (1 << bus);
	bool bus_initialized = bus_mask & _spi_bus_init_mask;

	// If bus is not initialized, initialize it
	if (!bus_initialized)
	{
		Spi::spi_bus_init(bus, freq);
		_spi_bus_init_mask |= bus_mask;
	}

	// Configure chip select
	pinMode(_chip_select, OUTPUT);
}

void Spi::transfer(uint8_t* send_buf, size_t ssize, uint8_t* recv_buf, size_t rsize)
{
	assert_chip_select();

	// Send all the bytes
	for (size_t i = 0; i < ssize; i++)
	{
		send_byte(send_buf[i]);
	}

	// Retrieve all the bytes
	for (size_t i = 0; i < rsize; i++)
	{
		recv_buf[i] = receive_byte();
	}

	deassert_chip_select();
}

// NOTE: caller is responsible for asserting CS
void Spi::send_byte(uint8_t byte)
{
	spi4teensy3::send(byte);
}

// NOTE: caller is responsible for asserting CS
uint8_t Spi::receive_byte(void)
{
	return spi4teensy3::receive();
}

// ----- Static members----- //
void Spi::spi_bus_init(uint8_t bus, unsigned frequency)
{
	spi4teensy3::init(bus, frequency);
}

uint8_t Spi::_spi_bus_init_mask = 0x00; // default no SPI busses initialized


} // end namespace interface