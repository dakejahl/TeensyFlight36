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

#include <ComplimentaryFilter.hpp>

void ComplimentaryFilter::apply(void)
{
	estimate_rpy_from_accel_and_gyro(_accel_xyz, _gyro_xyz);
}

void ComplimentaryFilter::estimate_rpy_from_accel_and_gyro(const Vector3f& accel, const Vector3f& gyro)
{
	// Calculate roll and pitch
	float roll = equations::roll_from_accel(accel.x(), accel.y(), accel.z());
	float pitch = equations::pitch_from_accel(accel.x(), accel.y(), accel.z());

	// Calculate roll and pitch rates
	float roll_rate = _gyro_xyz.x() + _gyro_xyz.y() * std::sin(_roll_est) * std::tan(_pitch_est)
					+ _gyro_xyz.z() * std::cos(_roll_est) * std::tan(_pitch_est);

	float pitch_rate = _gyro_xyz.y() * std::cos(_roll_est) - _gyro_xyz.z() * std::sin(_roll_est);


	auto now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();
	float dt = (now - _last_timestamp) / MICROS_PER_SEC;
	_last_timestamp = now;

	// Run filter
	_roll_est = (1 - _alpha) * (_roll_est + roll_rate * dt) + _alpha * roll;
	_pitch_est = (1 - _alpha) * (_pitch_est + pitch_rate * dt) + _alpha * pitch;

}