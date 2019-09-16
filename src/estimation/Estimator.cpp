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

#include <Estimator.hpp>

float Estimator::roll_from_quat(const Quaternionf& q)
{
	float q0 = q.w();
	float q1 = q.x();
	float q2 = q.y();
	float q3 = q.z();

	return std::atan((2*q0*q1 - 2*q2*q3) / (q0*q0 - q1*q1 + q2*q2 - q3*q3));
}

float Estimator::pitch_from_quat(const Quaternionf& q)
{
	float q0 = q.w();
	float q1 = q.x();
	float q2 = q.y();
	float q3 = q.z();

	return std::asin(2*q0*q3 + 2*q1*q2);
}

float Estimator::yaw_from_quat(const Quaternionf& q)
{
	float q0 = q.w();
	float q1 = q.x();
	float q2 = q.y();
	float q3 = q.z();

	return std::atan((2*q0*q2-2*q1*q3) / (q0*q0 + q1*q1 - q2*q2 - q3*q3));
}

void Estimator::apply_gyro_calibration(float& x, float& y, float& z)
{
	x = x - GYRO_OFFSET_X;
	y = y - GYRO_OFFSET_Y;
	z = z - GYRO_OFFSET_Z;
}

void Estimator::apply_accel_calibration(float& x, float& y, float& z)
{
	x = (x - ACCEL_OFFSET_X) * ACCEL_SCALE_X;
	y = (y - ACCEL_OFFSET_Y) * ACCEL_SCALE_Y;
	z = (z - ACCEL_OFFSET_Z) * ACCEL_SCALE_Z;
}

void Estimator::apply_mag_calibration(float& x, float& y, float& z)
{
	x = (x - MAG_OFFSET_X) / MAG_SCALE_X;
	y = (y - MAG_OFFSET_Y) / MAG_SCALE_Y;
	z = (z - MAG_OFFSET_Z) / MAG_SCALE_Z;
}

void Estimator::collect_sensor_data(void)
{
	if (_gyro_sub.updated())
	{
		auto data = _gyro_sub.get();
		auto x = data.x;
		auto y = data.y;
		auto z = data.z;

		apply_gyro_calibration(x, y, z);

		_gyro_xyz.x() = x;
		_gyro_xyz.y() = y;
		_gyro_xyz.z() = z;
	}

	if (_accel_sub.updated())
	{
		auto data = _accel_sub.get();
		auto x = data.x;
		auto y = data.y;
		auto z = data.z;

		apply_accel_calibration(x, y, z);

		_accel_xyz.x() = x;
		_accel_xyz.y() = y;
		_accel_xyz.z() = z;
	}

	if (_mag_sub.updated())
	{
		auto data = _mag_sub.get();
		auto x = data.x;
		auto y = data.y;
		auto z = data.z;

		apply_mag_calibration(x, y, z);

		_mag_xyz.x() = x;
		_mag_xyz.y() = y;
		_mag_xyz.z() = z;
	}
}