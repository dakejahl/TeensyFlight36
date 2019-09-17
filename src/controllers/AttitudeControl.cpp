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

#include <AttitudeControl.hpp>

AttitudeControl::AttitudeControl()
{
	_pwm = new Pwm(400);

	// NOTE: controller gains are from PX4
	float p = 6.5;
	float i = 0;
	float d = 0;
	float max_integrator = 0.3; // from PX4

	// Attitdue controllers are P controllers with anti integrator windup and reset mechanism.
	_pitch_controller = new controllers::PIDController(p, i, d, MAX_PITCH, max_integrator);
	_roll_controller = new controllers::PIDController(p, i, d, MAX_ROLL, max_integrator);

	// Rates controller -- full PID
	p = 0.15;
	i = 0.2;
	d = 0.003;
	max_integrator = 0.3;
	_pitch_rate_controller = new controllers::PIDController(p, i, d, MAX_PITCH, max_integrator);
	_roll_rate_controller = new controllers::PIDController(p, i, d, MAX_ROLL, max_integrator);
}

void AttitudeControl::collect_gyro_data(void)
{
	if (_gyro_sub.updated())
	{
		auto data = _gyro_sub.get();

		_gyro_x = data.x;
		_gyro_y = data.y;
		_gyro_z = data.z;
	}
}

void AttitudeControl::get_rc_input(void)
{
	if (_rc_sub.updated())
	{
		auto data = _rc_sub.get();

		_rc_throttle = data.channels[0]; // 982 -- 1514 -- 2006
		_rc_yaw = data.channels[3]; // 982 -- 1495 -- 2006
		_rc_pitch = data.channels[2]; // 982 -- 1494 -- 2006
		_rc_roll = data.channels[1]; // 982 -- 1493 -- 2006

		_rc_kill = data.channels[5]; // away: 982 -- center: 1494 -- towards me: 2006
	}
}

void AttitudeControl::check_for_arm_condition(void)
{
	if (_rc_throttle == 982 && _rc_yaw == 2006 && !_enabled)
	{
		auto now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();

		// Check if we just entered this logic
		if (_arm_timer == 0)
		{
			_arm_timer_start = now;
			_arm_timer = 1;
		}
		else
		{
			_arm_timer = now - _arm_timer_start;
		}

		if (_arm_timer > 2000000)
		{
			_enabled = true;
			_arm_timer = 0;
		}
	}
	else
	{
		_arm_timer = 0;
	}
}

void AttitudeControl::check_for_kill_condition(void)
{
	if (_rc_kill > KILL_VALUE)
	{
		_enabled = false;
	}
}

void AttitudeControl::convert_rc_to_trpy(void)
{
	// Throttle is scaled between 1 and 0
	_throttle_sp = (_rc_throttle - 982) / 982;

	// Roll/Pitch/Yaw is scaled between -1 and 1
	_pitch_sp = (_rc_pitch - 1495) / (0.5 * 982);
	_roll_sp = (_rc_roll - 1495) / (0.5 * 982);
}

void AttitudeControl::calculate_effort_for_rates_controller(void)
{

}

void AttitudeControl::convert_unit_rpy_to_rpy_degs(void)
{
	_pitch_sp = _pitch_sp * MAX_PITCH;
	_roll_sp = _roll_sp * MAX_ROLL;
}

void AttitudeControl::outputs_motors_idle(void)
{
	_pwm->write(pwm::MOTOR_1, pwm::IDLE_TRHOTTLE);
	_pwm->write(pwm::MOTOR_2, pwm::IDLE_TRHOTTLE);
	_pwm->write(pwm::MOTOR_3, pwm::IDLE_TRHOTTLE);
	_pwm->write(pwm::MOTOR_4, pwm::IDLE_TRHOTTLE);
}

void AttitudeControl::outputs_motors_disarmed(void)
{
	_pwm->write(pwm::MOTOR_1, pwm::MOTORS_DISARMED);
	_pwm->write(pwm::MOTOR_2, pwm::MOTORS_DISARMED);
	_pwm->write(pwm::MOTOR_3, pwm::MOTORS_DISARMED);
	_pwm->write(pwm::MOTOR_4, pwm::MOTORS_DISARMED);
}