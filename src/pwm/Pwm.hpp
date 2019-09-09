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

#include <board_config.hpp>

#include <Eigen/Dense>

namespace pwm
{
static constexpr unsigned MOTOR_1 = 3U;
static constexpr unsigned MOTOR_2 = 4U;
static constexpr unsigned MOTOR_3 = 16U;
static constexpr unsigned MOTOR_4 = 17U;

static constexpr unsigned MOTORS_DISARMED = 900U; // 900us
static constexpr unsigned IDLE_TRHOTTLE = 1150U; // 1150us
static constexpr unsigned FULL_TRHOTTLE = 1950U; //1950us

static constexpr float MICROS_PER_PWM_TICKS = 9.765625;

} // end namespace pwm

class Pwm
{
public:
	Pwm(unsigned frequency)
	{
		// Configure at 400Hz and hold low
		analogWriteFrequency(pwm::MOTOR_1, frequency);
		analogWrite(pwm::MOTOR_1, pwm::MOTORS_DISARMED);
	}

	void write(uint8_t motor, unsigned pulse_width_us)
	{
		int ticks = pulse_width_us / pwm::MICROS_PER_PWM_TICKS;
		// 2500us / 256 ticks == 9.765625

		analogWrite(motor, ticks);
	}
};

