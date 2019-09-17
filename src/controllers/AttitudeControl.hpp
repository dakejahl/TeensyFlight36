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

#include <PIDController.hpp>
#include <Pwm.hpp>
#include <Messenger.hpp>

#define ARM_TIME_US 2000000
#define KILL_VALUE 1000
#define MAX_PITCH 45
#define MAX_ROLL 45

class AttitudeControl
{
public:
	AttitudeControl();

	// Gyro stuff
	void collect_gyro_data(void);

	// RC stuff
	void get_rc_input(void);
	void check_for_arm_condition(void);
	void check_for_kill_condition(void);

	// Motor stuff
	bool armed(void) { return _enabled; };
	void outputs_motors_idle(void);
	void outputs_motors_disarmed(void);

	// Controller stuff
	void convert_rc_to_trpy(void);
	void convert_unit_rpy_to_rpy_degs(void);
	void calculate_effort_for_rates_controller(void);
private:

	// gyro filtered data (for control signals)
	float _gyro_x = 0;
	float _gyro_y = 0;
	float _gyro_z = 0;

	// Subscribers
	messenger::Subscriber<rc_input_s> _rc_sub;
	messenger::Subscriber<gyro_filtered_data_s> _gyro_sub;

	// PWM output module
	Pwm* _pwm {};

	// RC input from user
	uint16_t _rc_throttle = 0;
	uint16_t _rc_yaw = 0;
	uint16_t _rc_pitch = 0;
	uint16_t _rc_roll = 0;
	uint16_t _rc_kill = 0;

	// Attitude control
	float _throttle_sp = 0;
	float _roll_sp = 0;
	float _pitch_sp = 0;
	float _yaw_sp = 0;

	controllers::PIDController* _pitch_controller;
	controllers::PIDController* _roll_controller;

	// Rate control
	controllers::PIDController* _pitch_rate_controller;
	controllers::PIDController* _roll_rate_controller;

	// arming stuff
	bool _enabled = false;
	abs_time_t _arm_timer = 0;
	abs_time_t _arm_timer_start = 0;
};