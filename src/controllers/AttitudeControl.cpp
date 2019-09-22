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

	//----- Rate controller settings -----//
	// pitch
	float p = 0.07; // 0.3 is is oscillations
	float i = 0.02;
	float d = 2; // 6 causes oscillations, so we turn down by 2/3
	float max_effort = 1; // roll pitch and yaw are scaled from -1 to 1
	float max_integrator = 0.3; // 30% of output
	_pitch_rate_controller = new controllers::PIDController(p, i, d, max_effort, max_integrator);

	// roll


	// _roll_rate_controller = new controllers::PIDController(p, i, d, max_effort, max_integrator);


	//----- Attitude controller settings -----//
	// pitch
	p = 1;
	i = 0;
	d = 0;
	max_effort = MAX_ANGULAR_RATE_RAD;
	_pitch_controller = new controllers::PIDController(p, i, d, max_effort, 0);

	// roll

}
void AttitudeControl::collect_attitude_data(void)
{
	if (_attitude_sub.updated())
	{
		auto data = _attitude_sub.get();

		_roll = data.roll;
		_pitch = data.pitch;
	}
}

void AttitudeControl::collect_attitude_rate_data(void)
{
	if (_gyro_sub.updated())
	{
		auto data = _gyro_sub.get();

		float x = data.x;
		float y = data.y;
		float z = data.z;

		_pitch_rate = equations::pitch_rate_from_gyro(x, y, z, _roll);
		_roll_rate = equations::roll_rate_from_gyro(x, y, z, _roll, _pitch);

		// Publish this rate data
		rates_control_euler_s signal;
		signal.roll = _roll_rate;
		signal.pitch = _pitch_rate;

		_rates_control_pub.publish(signal);
	}
}

void AttitudeControl::get_rc_input(void)
{
	if (_manual_control_sub.updated())
	{
		auto data = _manual_control_sub.get();

		_rc_throttle = data.throttle;
		_rc_yaw = data.yaw;
		_rc_pitch = data.pitch;
		_rc_roll = data.roll;
		_rc_kill = data.kill_switch;
	}
}

void AttitudeControl::convert_sticks_to_setpoints(void)
{
	// pitch and roll are fed into the attitude controller
	_pitch_sp = _rc_pitch * MAX_PITCH_ANGLE_RAD;
	_roll_sp = _rc_roll * MAX_ROLL_ANGLE_RAD;

	_yaw_rate_sp = _rc_yaw * MAX_ANGULAR_RATE_RAD; // yaw is fed into only a rates controller

	_throttle_sp = _rc_throttle; // throttle_sp is fed directly to the mixer
}

void AttitudeControl::check_for_arm_condition(void)
{
	if (_rc_throttle == 0 && _rc_yaw == -1 && !_enabled)
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
	if (_rc_kill)
	{
		_enabled = false;
	}
}

void AttitudeControl::run_controllers(void)
{

	// ----- ROLL -----/
	// Attitude
	// float roll_rate_sp = _roll_controller->get_effort(_roll_sp, _roll);
	// Rates
	// float roll_effort = _roll_rate_controller->get_effort(roll_rate_sp, _roll_rate);

	// Scale actuator effort by the

	// ----- PITCH -----/
	// Attitude
	// float pitch_rate_sp = _pitch_controller->get_effort(_pitch_sp, _pitch);


	// TESTING CONTROL AROUND ZERO


	//some hacky logic to induce oscillations
	// float degs = 5;
	// float angle = degs * M_PI / 180;
	// if (_pitch > angle)
	// {
	// 	float dps = -80;
	// 	pitch_rate_sp = dps * M_PI / 180;
	// }
	// else if (_pitch < -angle)
	// {
	// 	float dps = 80;
	// 	pitch_rate_sp = dps * M_PI / 180;
	// }

	// pitch_rate_sp = 0;

	float angle_sp = 20 * M_PI / 180;

	setpoint_angle_s angle_sp_data;
	angle_sp_data.pitch = angle_sp;
	_angle_sp_pub.publish(angle_sp_data);

	// // ----- Attitude Control ----- //
	pitch_rate_sp = _pitch_controller->get_effort(angle_sp, _pitch);

	// // publish the rates setpoint
	setpoint_rates_s rates_sp;
	rates_sp.pitch = pitch_rate_sp;
	_rates_sp_pub.publish(rates_sp);

	// ----- Rate Control ----- //
	float pitch_effort = _pitch_rate_controller->get_effort(pitch_rate_sp, _pitch_rate);




	// unsigned motor_effort_1 = pwm::IDLE_THROTTLE + (pwm::SAFE_THROTTLE - pwm::IDLE_THROTTLE) * (_throttle_sp - pitch_effort - roll_effort);
	// unsigned motor_effort_2 = pwm::IDLE_THROTTLE + (pwm::SAFE_THROTTLE - pwm::IDLE_THROTTLE) * (_throttle_sp + pitch_effort + roll_effort);
	// unsigned motor_effort_3 = pwm::IDLE_THROTTLE + (pwm::SAFE_THROTTLE - pwm::IDLE_THROTTLE) * (_throttle_sp - pitch_effort + roll_effort);
	// unsigned motor_effort_4 = pwm::IDLE_THROTTLE + (pwm::SAFE_THROTTLE - pwm::IDLE_THROTTLE) * (_throttle_sp + pitch_effort - roll_effort);

	// Just a pitch controller
	unsigned motor_effort_1 = pwm::IDLE_THROTTLE + (pwm::SAFE_THROTTLE - pwm::IDLE_THROTTLE) * (_throttle_sp - pitch_effort);
	unsigned motor_effort_2 = pwm::IDLE_THROTTLE + (pwm::SAFE_THROTTLE - pwm::IDLE_THROTTLE) * (_throttle_sp + pitch_effort);
	unsigned motor_effort_3 = pwm::IDLE_THROTTLE + (pwm::SAFE_THROTTLE - pwm::IDLE_THROTTLE) * (_throttle_sp - pitch_effort);
	unsigned motor_effort_4 = pwm::IDLE_THROTTLE + (pwm::SAFE_THROTTLE - pwm::IDLE_THROTTLE) * (_throttle_sp + pitch_effort);

	motor_effort_1 = equations::clamp<unsigned>(motor_effort_1, pwm::IDLE_THROTTLE, pwm::SAFE_THROTTLE);
	motor_effort_2 = equations::clamp<unsigned>(motor_effort_2, pwm::IDLE_THROTTLE, pwm::SAFE_THROTTLE);
	motor_effort_3 = equations::clamp<unsigned>(motor_effort_3, pwm::IDLE_THROTTLE, pwm::SAFE_THROTTLE);
	motor_effort_4 = equations::clamp<unsigned>(motor_effort_4, pwm::IDLE_THROTTLE, pwm::SAFE_THROTTLE);

	// Ouput it to the motors
	_pwm->write(pwm::MOTOR_1, motor_effort_1);
	_pwm->write(pwm::MOTOR_2, motor_effort_2);
	_pwm->write(pwm::MOTOR_3, motor_effort_3);
	_pwm->write(pwm::MOTOR_4, motor_effort_4);
}

void AttitudeControl::outputs_motors_disarmed(void)
{
	_pwm->write(pwm::MOTOR_1, pwm::MOTORS_DISARMED);
	_pwm->write(pwm::MOTOR_2, pwm::MOTORS_DISARMED);
	_pwm->write(pwm::MOTOR_3, pwm::MOTORS_DISARMED);
	_pwm->write(pwm::MOTOR_4, pwm::MOTORS_DISARMED);
}