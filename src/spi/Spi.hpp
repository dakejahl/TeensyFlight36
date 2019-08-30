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

#include <core_pins.h>
#include <spi4teensy3.hpp>

namespace interface
{

class Spi
{
public:

	Spi(uint8_t bus, unsigned frequency, uint8_t chip_select);

	void transfer(uint8_t* send_buf, size_t ssize, uint8_t* recv_buf, size_t rsize);

	void assert_chip_select(void) { digitalWrite(_chip_select, LOW); };
	void deassert_chip_select(void){ digitalWrite(_chip_select, HIGH); };

private:
	// ----- Instance ----- //
	void send_byte(uint8_t byte);
	uint8_t receive_byte(void);

	uint8_t _chip_select = 0;

	// ----- Static ----- //
	// TODO: implement support for more than SPI_0. Right now the spi4teensy3 lib just
	// initializes SPI_0
	// Configures registers to correctly initialize SPI_0
	static void spi_bus_init(uint8_t bus, unsigned frequency);

	static uint8_t _spi_bus_init_mask;

};

} // end namespace interface
