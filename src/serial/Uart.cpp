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

#include  <Uart.hpp>

extern "C" void uart0_isr_hook(void)
{
	auto saved_state = taskENTER_CRITICAL_FROM_ISR();

	interface::Uart* instance = interface::Uart::Instance(0);
	if (instance != nullptr)
	{
		// Notify the task using this UART that data is ready
		bool registered = instance->interrupt_callback_registered();
		if (registered)
		{
			instance->callback();
		}
	}

	taskEXIT_CRITICAL_FROM_ISR(saved_state);
}

namespace interface
{

Uart* Uart::Instantiate(uint8_t uart, unsigned baud, unsigned format)
{
	if (_instances[uart] == nullptr)
	{
		_instances[uart] = new Uart(uart, baud, format);
	}

	return _instances[uart];
}

Uart* Uart::Instance(uint8_t uart)
{
	switch (uart)
	{
		case 0:
			return _instances[uart];
			break;

		default:
			// TODO: support other uarts
			break;
	}

	return nullptr;
}

Uart::Uart(uint8_t uart, unsigned baud, unsigned format)
{
	switch (uart)
	{
		case 0:
			// comes out to 8.333kB/s (interrupt per byte would make that 8.33kHz interrupt which is okayish)
			Serial1.begin(baud, format); // TODO: fix hack inside of serial1.c (hardcodes 2 stop bits on UART)
			break;

		default:
			// TODO: support other uarts
			break;
	}
}

uint8_t Uart::read(void)
{
	return Serial1.read();
}

bool Uart::data_available(void)
{
	return Serial1.available();
}

Uart* Uart::_instances[MAX_UARTS];

} // end namespace interface