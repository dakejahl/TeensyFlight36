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

static constexpr uint8_t UART_BUS_0 = 0;
static constexpr unsigned BAUD_100k = 100000;

Sbus::Sbus(TaskHandle_t& handle)
	: _task_handle(handle)
{
	// Interrupts need to be disabled while we configure the uart isr
	taskENTER_CRITICAL();

	NVIC_DISABLE_IRQ(IRQ_UART0_STATUS);

	// Configure UART0 as 8E2
	// 100'000 baud rate
	// invert RX signalling (SBUS)
	// 1 start bit
	// 8 data bits
	// 1 even parity bit
	// 2 stop bits
	_uart = Uart::Instantiate(UART_BUS_0, BAUD_100k, SERIAL_8E1_RXINV);

	_uart->register_interrupt_callback<Sbus>(this);

	// WARNING: FreeRTOS is like "if your priority is higher (lower number) than 80 .. then fuck you"
	NVIC_SET_PRIORITY(IRQ_UART0_STATUS, 240); // Cortex-M4: 0,16,32,48,64,80,96,112,128,144,160,176,192,208,224,240

	NVIC_ENABLE_IRQ(IRQ_UART0_STATUS); // only enable the interrupt after everything is configured correctly

	taskEXIT_CRITICAL();
}

void Sbus::interrupt_callback(void)
{
	//  We do not need this functionality at the momemt
}

void Sbus::collect_data(void)
{
	uint8_t sbus_frame[25] = {};

	// error counter to count the lost frame
	int error_count = 0;

	while (1)
	{
		int bytes_read = 0;

		// We break from this loop in 2 conditions:
		// 1. We get 25 bytes of data
		// 2. We have no more data to process
		for (size_t i = 0; _uart->data_available() && (i < sizeof(sbus_frame)/sizeof(sbus_frame[0])); i++)
		{
			sbus_frame[i] = _uart->read();
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


	// DEBUGGING
	auto print_data = [channels_data, error_count]
	{
		for (size_t i = 0; i < sizeof(channels_data)/sizeof(channels_data[0]); i++)
		{
			SYS_INFO("channels_data[%d]: %d", i, channels_data[i]);
		}
		SYS_INFO("errors: %d", error_count);
		SYS_INFO("--- --- --- --- --- ---");
	};

	// print_data();

}

} // end namespace interface
