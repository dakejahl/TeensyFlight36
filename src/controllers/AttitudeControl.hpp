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
#include <Equations.hpp>

#define ARM_TIME_US 2000000
#define MAX_PITCH_ANGLE_RAD 30 * M_PI / 180
#define MAX_ROLL_ANGLE_RAD 30* M_PI / 180
#define MAX_ANGULAR_RATE_RAD 720 * M_PI / 180
#define MAX_YAW_RATE_RAD 180 * M_PI / 180

class AttitudeControl
{
public:
	AttitudeControl();

	// Gyro stuff
	void collect_attitude_data(void);
	void collect_attitude_rate_data(void);

	// RC stuff
	void get_rc_input(void);
	void check_for_arm_condition(void);
	void check_for_kill_condition(void);

	// Motor stuff
	bool armed(void) { return _enabled; };
	void outputs_motors_disarmed(void);

	// Controller stuff
	void convert_sticks_to_setpoints(void);

	// Runs the attitude controller and then the rates controller
	void run_controllers(void);

private:
	// Attitude
	float _roll = 0;
	float _pitch = 0;
	float _yaw = 0;

	// Rates
	float _roll_rate = 0;
	float _pitch_rate = 0;
	float _yaw_rate = 0;

	// Subscribers
	messenger::Subscriber<manual_control_s> _manual_control_sub;
	messenger::Subscriber<gyro_filtered_data_s> _gyro_sub;
	messenger::Subscriber<attitude_euler_s> _attitude_sub;

	// Setpoint publishers
	messenger::Publisher<rates_control_euler_s> _rates_control_pub;
	messenger::Publisher<setpoint_rates_s> _rates_sp_pub;
	messenger::Publisher<setpoint_angle_s> _angle_sp_pub;


	// PWM output module
	Pwm* _pwm {};

	// RC input from user
	float _rc_throttle = 0;
	float _rc_yaw = 0;
	float _rc_pitch = 0;
	float _rc_roll = 0;
	bool _rc_kill = false;

	// Attitude control
	float _throttle_sp = 0;
	float _roll_sp = 0;
	float _pitch_sp = 0;

	controllers::PIDController* _pitch_controller;
	controllers::PIDController* _roll_controller;

	// Rate control
	float _yaw_rate_sp = 0;

	controllers::PIDController* _pitch_rate_controller;
	controllers::PIDController* _roll_rate_controller;
	controllers::PIDController* _yaw_rate_controller;

	// arming stuff
	bool _enabled = false;
	abs_time_t _arm_timer = 0;
	abs_time_t _arm_timer_start = 0;
};