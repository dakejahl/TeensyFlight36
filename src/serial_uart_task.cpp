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
#include <Messenger.hpp>
#include <Sbus.hpp>

// TODO:
struct input_rc_s
{
	uint64_t timestamp;
	uint64_t timestamp_last_signal;
	uint32_t channel_count;
	int32_t rssi;
	uint16_t rc_lost_frame_count;
	uint16_t rc_total_frame_count;
	uint16_t rc_ppm_frame_length;
	uint16_t values[18];
	bool rc_failsafe;
	bool rc_lost;
	uint8_t input_source;
	uint8_t _padding0[3]; // required for logger
};

void serial_uart_task(void* args)
{
	auto handle = xTaskGetCurrentTaskHandle();
	auto sbus = new interface::Sbus(handle);

	for(;;)
	{
		// TODO: make sbus task less stupid
		// The vTaskDelay() call is inside of the class....
		sbus->collect_data();
	}
}