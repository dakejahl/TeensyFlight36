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

#include <Eigen/Dense>
#include <cmath>

#include <Time.hpp>
#include <board_config.hpp>

using namespace Eigen;

class AttitudeEstimator
{
public:
	AttitudeEstimator()
	{

	}

	Quaternionf calculate_quaternions();

private:
	// Functions
	Quaternionf Dav_qmethod(Vector3f r1, Vector3f r2, Vector3f b1, Vector3f b2, float a1, float a2);

	Quaternionf quat_error(Quaternionf q1, Quaternionf q2);

	Quaternionf vect2q(Vector4f v);

	Vector4f q2vect(Quaternionf q);

	Quaternionf qexp(Quaternionf q);

	Matrix3f crossp_mat(Vector3f v);

	template <typename T> int sgn(T val)
	{
    	return (T(0) < val) - (val < T(0));
	}

	// Variables
	abs_time_t _last_time = 0;

	Quaternionf _q_obs = {};
	Quaternionf _q_err = {};

	Vector3f _bias_obs = {};

	Vector3f _epsilon_err = {};
	float _eta_err = {};


	Quaternionf _unknown_variable = {};



};