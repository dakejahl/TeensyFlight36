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

namespace mpu9250_spi
{

static constexpr uint8_t CS = 10;
static constexpr uint8_t BUS = 0;
static constexpr unsigned FREQ = 8000000;

} // end namespace mpu9250_spi

namespace address
{

// Configuration
static constexpr uint8_t SMPLRT_DIV = 25;
static constexpr uint8_t CONFIG = 26;

// Sensors
static constexpr uint8_t GYRO_CONFIG = 27;
static constexpr uint8_t ACCEL_CONFIG = 28;
static constexpr uint8_t ACCEL_CONFIG_2 = 29;

// Interrupts
static constexpr uint8_t INT_ENABLE = 56;

// Power management
static constexpr uint8_t PWR_MGMT_1 = 107;
static constexpr uint8_t PWR_MGMT_2 = 108;
static constexpr uint8_t USER_CTRL = 106;

// Testing
static constexpr uint8_t WHOAMI = 117; // 0111 0101

} // end namespace address

namespace value
{

// Testing
static constexpr uint8_t WHOAMI = 0x71;
// Power management
static constexpr uint8_t CLK_SEL_AUTO = 0x01; // Auto selects the best available clock source -- PLL if ready, else use the Internal oscillator.
static constexpr uint8_t ENABLE_ACCEL_GYRO = 0x00; // Enables XYZ in both accel and gyro -- No sleep modes obviously.
// User control
static constexpr uint8_t BIT_I2C_IF_DIS = 0x10; // Disable I2C Slave module and put the serial interface in SPI mode only.
static constexpr uint8_t BIT_SIG_COND_RST = 0x01; // Reset all gyro digital signal path, accel digital signal path, and temp digital signal path. This bit also clears all the sensor registers.
// Interrupt control
static constexpr uint8_t INT_DISABLE = 0x00; // Disable I2C Slave module and put the serial interface in SPI mode only.
// Sample rate divider
static constexpr uint8_t SMPLRT_DIV_NONE  = 0x00; // NOTE: This register is only effective when DLPF is being used.
// Config
static constexpr uint8_t CONFIG_NO_DLPF_GYRO_3600Hz = 0b000000010; // Gyro specs --> 32kHz sample rate    Bandwidth: 3600Hz    Delay: 0.11ms
// Gyro
static constexpr uint8_t GYRO_NO_DLPF_2000_DPS = 0b00011010;
// Accel
static constexpr uint8_t ACCEL_16_G = 0b00011000; // +/- 16g
static constexpr uint8_t ACCEL_NO_DLPF_4kHz = 0b00001000; // Accel specs --> 4kHz sample rate    Bandwidth: 1046Hz    Delay: 0.503ms

} // end namespace value

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

private:

	interface::Spi* _interface;

};