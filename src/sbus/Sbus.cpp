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

#include <Sbus.hpp>

namespace interface
{

Sbus::Sbus()
{
	// Configure UART0 as 8E2
	// 100'000 baud rate
	// invert RX signalling (SBUS)
	// 1 start bit
	// 8 data bits
	// 1 even parity bit
	// 2 stop bits

	// comes out to 8.333kB/s (interrupt per byte would make that 8.33kHz interrupt which is okayish)
	Serial1.begin(100000, SERIAL_8E1_RXINV); // TODO: fix hack inside of serial1.c (hardcodes 2 stop bits on UART0)
}

void Sbus::collect_data(void)
{
	uint8_t sbus_frame[25] = {};

	// error counter to count the lost frame
	int error_count = 0;
	while (1)
	{
		int bytes_read = 0;

		// We break from this loop in 2 contions:
		// 1. We get 25 bytes of data
		// 2. We have no more data to process
		for (size_t i = 0; Serial1.available() && (i < sizeof(sbus_frame)/sizeof(sbus_frame[0])); i++)
		{
			sbus_frame[i] = Serial1.read();
			bytes_read++;
		}

		// If we got all 25 bytes -- we move forward
		if (25 == bytes_read)
		{
			// Notice: most sbus rx device support sbus1
			if (0x0f == sbus_frame[0] && 0x00 == sbus_frame[24])
			{
				break;
			}
		}

		++error_count;

		vTaskDelay(5);
	}

	 // Parse SBUS and convert to PWM
	int channels_data[16];
	channels_data[0] = (uint16_t)(((sbus_frame[1] | sbus_frame[2] << 8) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[1] = (uint16_t)(((sbus_frame[2] >> 3 | sbus_frame[3] << 5) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[2] = (uint16_t)(((sbus_frame[3] >> 6 | sbus_frame[4] << 2 | sbus_frame[5] << 10) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[3] = (uint16_t)(((sbus_frame[5] >> 1 | sbus_frame[6] << 7) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[4] = (uint16_t)(((sbus_frame[6] >> 4 | sbus_frame[7] << 4) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[5] = (uint16_t)(((sbus_frame[7] >> 7 | sbus_frame[8] << 1 | sbus_frame[9] << 9) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[6] = (uint16_t)(((sbus_frame[9] >> 2 | sbus_frame[10] << 6) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[7] = (uint16_t)(((sbus_frame[10] >> 5 | sbus_frame[11] << 3) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET; // & the other 8 + 2 channels if you need them
	channels_data[8] = (uint16_t)(((sbus_frame[12] | sbus_frame[13] << 8) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[9] = (uint16_t)(((sbus_frame[13] >> 3 | sbus_frame[14] << 5) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[10] = (uint16_t)(((sbus_frame[14] >> 6 | sbus_frame[15] << 2 | sbus_frame[16] << 10) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[11] = (uint16_t)(((sbus_frame[16] >> 1 | sbus_frame[17] << 7) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[12] = (uint16_t)(((sbus_frame[17] >> 4 | sbus_frame[18] << 4) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[13] = (uint16_t)(((sbus_frame[18] >> 7 | sbus_frame[19] << 1 | sbus_frame[20] << 9) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[14] = (uint16_t)(((sbus_frame[20] >> 2 | sbus_frame[21] << 6) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	channels_data[15] = (uint16_t)(((sbus_frame[21] >> 5 | sbus_frame[22] << 3) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;

	print_formatted_data(channels_data, 16);
}

void Sbus::print_formatted_data(int* buffer, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		SYS_INFO("channels_data[%d]: %d", i, buffer[i]);
	}
	SYS_INFO("--- --- --- --- --- ---")
}

} // end namespace interface