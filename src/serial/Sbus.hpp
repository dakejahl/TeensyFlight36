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

#include <cmath>

#include <board_config.hpp>
#include <Uart.hpp>
#include <Messenger.hpp>
#include <Equations.hpp>


#define RC_KILL_VALUE 1000

namespace interface
{
// define range mapping here, -+100% -> 1000..2000
static constexpr unsigned SBUS_FRAME_SIZE = 25;
static constexpr unsigned RC_NUMBER_CHANNELS = 16;

static constexpr float SBUS_RANGE_MIN = 200.0f;
static constexpr float SBUS_RANGE_MAX = 1800.0f;
static constexpr float SBUS_TARGET_MIN = 1000.0f;
static constexpr float SBUS_TARGET_MAX = 2000.0f;
// pre-calculate the floating point stuff as far as possible at compile time
static constexpr float SBUS_SCALE_FACTOR = ((SBUS_TARGET_MAX - SBUS_TARGET_MIN) / (SBUS_RANGE_MAX - SBUS_RANGE_MIN));
static constexpr float SBUS_SCALE_OFFSET = (int)(SBUS_TARGET_MIN - (SBUS_SCALE_FACTOR * SBUS_RANGE_MIN + 0.5f));

class Sbus
{
public:

	Sbus(TaskHandle_t& handle);

	void collect_data(void);

	void apply_deadzone(float& roll, float& pitch, float& yaw);
	void apply_expo(float& roll, float& pitch, float& yaw);

	void publish_data(abs_time_t& timestamp);

	void interrupt_callback(void);

	void print_data(void);

private:

	uint8_t _sbus_frame[SBUS_FRAME_SIZE] = {};
	int _lost_frames = 0;
	bool _rc_failsafe = false;
	bool _rc_lost = false;

	int _channels_data[RC_NUMBER_CHANNELS] = {};

	interface::Uart0* _uart;
	TaskHandle_t _task_handle; // handle of the task that owns this interface

	messenger::Publisher<manual_control_s> _manual_control_pub;
};

} // end namespace interface