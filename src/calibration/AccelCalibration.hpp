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

#include <algorithm>

static constexpr float BIG_ENOUGH = 9.0f;
static constexpr float GRAVITY_ACCEL = 9.80665f;

enum class CalibrationSide
{
	UPSIDE_UP = 0,
	UPSIDE_DOWN,
	LEFT_SIDE,
	RIGHT_SIDE,
	NOSE_UP,
	NOSE_DOWN,
	UNKNOWN = 255
};

class AccelCalibration
{
public:

	CalibrationSide get_next_side_to_calibrate(void);

	void calibrate(CalibrationSide side);

	void accumlate_data_for_side(CalibrationSide side);

	void update_measured_g_for_side(CalibrationSide side, unsigned num_samples);

	void calculate_offsets_and_scales(void);

	bool all_sides_complete(void)
	{
		return 	_upside_up_calibrated && _upside_down_calibrated &&
				_left_side_calibrated && _right_side_calibrated &&
				_nose_up_calibrated && _nose_down_calibrated;
	}

private:
	messenger::Subscriber<accel_raw_data_s> _accel_sub;

	accel_raw_data_s _data;

	// flags
	bool _upside_up_calibrated = false;
	bool _upside_down_calibrated = false;

	bool _left_side_calibrated = false;
	bool _right_side_calibrated = false;

	bool _nose_up_calibrated = false;
	bool _nose_down_calibrated = false;

	// stores accumulated data before averaging
	double _accumulate = 0;

	// Measured gravity vectory on each side
	float _upside_up_measured_g = 0;
	float _upside_down_measured_g = 0;

	float _left_side_measured_g = 0;
	float _right_side_measured_g = 0;

	float _nose_up_measured_g = 0;
	float _nose_down_measured_g = 0;

	// offsets
	float _x_offset = 0;
	float _y_offset = 0;
	float _z_offset = 0;

	// scales
	float _x_scale = 0;
	float _y_scale = 0;
	float _z_scale = 0;

};