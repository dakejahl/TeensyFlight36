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

static constexpr uint8_t MPU9250_SPI_CS = 10;
static constexpr uint8_t MPU9250_SPI_BUS = 0;
static constexpr uint8_t MPU9250_SPI_FREQ = 8000000;

namespace address
{
static constexpr uint8_t WHOAMI = 0x75; // 0111 0101
} // end namespace register

namespace value
{
	static constexpr uint8_t WHOAMI = 0x71;
}

class Mpu9250
{
public:

	Mpu9250()
	{
		// Initialize the SPI interface
		_interface = new interface::Spi(MPU9250_SPI_BUS, MPU9250_SPI_FREQ, MPU9250_SPI_CS);
	}

	bool probe(void);

	uint8_t read_register(uint8_t reg);

private:

	interface::Spi* _interface;

};