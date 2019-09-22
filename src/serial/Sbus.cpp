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
	_uart = Uart0::Instantiate(UART_BUS_0, BAUD_100k, SERIAL_8E1_RXINV);

	_uart->register_interrupt_callback<Sbus>(this);

	// WARNING: FreeRTOS is like "if your priority is higher (lower number) than 80 .. then fuck you"
	NVIC_SET_PRIORITY(IRQ_UART0_STATUS, 240); // Cortex-M4: 0,16,32,48,64,80,96,112,128,144,160,176,192,208,224,240


	// TODO: enable ISR callbacks for data processing? Frame spacing is 5ms (200Hz) which is pretty slow, this is probably unnecessary.
	// NVIC_ENABLE_IRQ(IRQ_UART0_STATUS); // only enable the interrupt after everything is configured correctly

	taskEXIT_CRITICAL();
}

void Sbus::interrupt_callback(void)
{
	//  We do not need this functionality at the momemt
}

void Sbus::collect_data(void)
{
	while (1)
	{
		int bytes_read = 0;

		// We break from this loop in 2 conditions:
		// 1. We get 25 bytes of data
		// 2. We have no more data to process
		for (size_t i = 0; _uart->data_available() && (i < sizeof(_sbus_frame)/sizeof(_sbus_frame[0])); i++)
		{
			_sbus_frame[i] = _uart->read();
			bytes_read++;
		}

		// If we got all 25 bytes -- we move forward
		if (bytes_read == SBUS_FRAME_SIZE)
		{
			// Notice: most sbus rx device support sbus1
			if (0x0f == _sbus_frame[0] && 0x00 == _sbus_frame[24])
			{
				break;
			}
		}
		// Otherwise we increment the error counter and try again
		else
		{
			++_lost_frames;
			vTaskDelay(5);
		}
	}

	 // Parse SBUS and convert to PWM
	_channels_data[0] = (uint16_t)(((_sbus_frame[1] | _sbus_frame[2] << 8) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[1] = (uint16_t)(((_sbus_frame[2] >> 3 | _sbus_frame[3] << 5) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[2] = (uint16_t)(((_sbus_frame[3] >> 6 | _sbus_frame[4] << 2 | _sbus_frame[5] << 10) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[3] = (uint16_t)(((_sbus_frame[5] >> 1 | _sbus_frame[6] << 7) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[4] = (uint16_t)(((_sbus_frame[6] >> 4 | _sbus_frame[7] << 4) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[5] = (uint16_t)(((_sbus_frame[7] >> 7 | _sbus_frame[8] << 1 | _sbus_frame[9] << 9) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[6] = (uint16_t)(((_sbus_frame[9] >> 2 | _sbus_frame[10] << 6) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[7] = (uint16_t)(((_sbus_frame[10] >> 5 | _sbus_frame[11] << 3) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET; // & the other 8 + 2 channels if you need them
	_channels_data[8] = (uint16_t)(((_sbus_frame[12] | _sbus_frame[13] << 8) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[9] = (uint16_t)(((_sbus_frame[13] >> 3 | _sbus_frame[14] << 5) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[10] = (uint16_t)(((_sbus_frame[14] >> 6 | _sbus_frame[15] << 2 | _sbus_frame[16] << 10) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[11] = (uint16_t)(((_sbus_frame[16] >> 1 | _sbus_frame[17] << 7) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[12] = (uint16_t)(((_sbus_frame[17] >> 4 | _sbus_frame[18] << 4) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[13] = (uint16_t)(((_sbus_frame[18] >> 7 | _sbus_frame[19] << 1 | _sbus_frame[20] << 9) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[14] = (uint16_t)(((_sbus_frame[20] >> 2 | _sbus_frame[21] << 6) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;
	_channels_data[15] = (uint16_t)(((_sbus_frame[21] >> 5 | _sbus_frame[22] << 3) & 0x07FF) * SBUS_SCALE_FACTOR + .5f) + SBUS_SCALE_OFFSET;

	_rc_failsafe = (_sbus_frame[23] & (1 << 3)) ? true : false;
	_rc_lost = (_sbus_frame[23] & (1 << 2)) ? true : false;
}

void Sbus::publish_data(abs_time_t& timestamp)
{
	float throttle = _channels_data[0];
	float yaw = _channels_data[3];
	float pitch = _channels_data[2];
	float roll = _channels_data[1];
	float kill = _channels_data[5];

	// Scale the sticks
	static constexpr float stick_high = 2006;
	static constexpr float stick_low = 982;
	static constexpr float stick_center = 1495;
	static constexpr float stick_range = stick_high - stick_low;

	throttle = (throttle - stick_low) / (stick_range); // Throttle is scaled between 0 and 1
	yaw = -(yaw - stick_center) / (0.5*stick_range); // Yaw is negative because positive yaw is to the right
	pitch = (pitch - stick_center) / (0.5*stick_range); // Roll/Pitch/Yaw is scaled between -1 and 1
	roll = (roll - stick_center) / (0.5*stick_range);

	// We only want ticks of 0.01 with range of -1 to 1 -- so we round these down
	unsigned round_to_hundreds = 100U;
	throttle = std::round(throttle * round_to_hundreds) / round_to_hundreds;
	yaw = std::round(yaw * round_to_hundreds) / round_to_hundreds;
	pitch = std::round(pitch * round_to_hundreds) / round_to_hundreds;
	roll = std::round(roll * round_to_hundreds) / round_to_hundreds;

	// Publish scaled sticks
	manual_control_s control;

	control.roll = roll;
	control.pitch = pitch;
	control.yaw = yaw;
	control.throttle = throttle;
	control.kill_switch = kill > RC_KILL_VALUE ? 1 : 0;

	_manual_control_pub.publish(control);
}

void Sbus::print_data(void)
{
	SYS_INFO("failsafe: %d", _rc_failsafe);
	SYS_INFO("rc_lost: %d", _rc_lost);

	for (size_t i = 0; i < RC_NUMBER_CHANNELS; i++)
	{
		SYS_INFO("channels_data[%d]: %d", i, _channels_data[i]);
	}
	SYS_INFO("--- --- --- --- --- ---");
}

} // end namespace interface
