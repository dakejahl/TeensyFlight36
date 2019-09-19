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

namespace equations
{

inline float roll_from_accel(float x, float y, float z)
{
	return std::atan( y / (std::sqrt(x*x + z*z))) * 180.0f / M_PI;
}

inline float pitch_from_accel(float x, float y, float z)
{
	return std::atan( x / (std::sqrt(y*y + z*z))) * 180.0f / M_PI;
}

inline float roll_rate_from_gyro(float x, float y, float z, float pitch_est, float roll_est)
{
	float roll_rate = x + y * std::sin(roll_est) * std::tan(pitch_est)
					+ z * std::cos(roll_est) * std::tan(pitch_est);

	return roll_rate;
}

inline float pitch_rate_from_gyro(float x, float y, float z, float roll_est)
{
	float pitch_rate = y * std::cos(roll_est) - z * std::sin(roll_est);

	return pitch_rate;
}

inline float yaw_rate_from_gyro(float x, float y, float z, float pitch_est, float roll_est)
{
	float yaw_rate = y * std::sin(roll_est) / std::cos(pitch_est) + z * std::cos(roll_est) / std::cos(pitch_est);

	return yaw_rate;
}

template <typename T>
inline float clamp(T val, T lo, T hi)
{
	if (val > hi)
	{
		val = hi;
	}
	else if (val < lo)
	{
		val = lo;
	}

	return val;
}

}; // end namespace estimation