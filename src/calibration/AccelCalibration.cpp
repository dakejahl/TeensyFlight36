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

#include <board_config.hpp>
#include <Messenger.hpp>
#include <AccelCalibration.hpp>

CalibrationSide AccelCalibration::get_next_side_to_calibrate(void)
{
	while (1)
	{
		_data = _accel_sub.get();
		unsigned stable_count = 0; // we must stay above BIG_ENOUGH FOR atleast 2 second continuously

		// we are nose up
		if (_data.x > BIG_ENOUGH && !_nose_up_calibrated)
		{

			while (_data.x > BIG_ENOUGH)
			{
				stable_count++;
				if (stable_count > 200)
				{
					SYS_INFO("Nose UP detected");
					return CalibrationSide::NOSE_UP;
				}

				vTaskDelay(10);
				_data = _accel_sub.get();
			}
		}
		// we are nose down
		else if (_data.x < -BIG_ENOUGH && !_nose_down_calibrated)
		{
			while (_data.x < -BIG_ENOUGH)
			{
				stable_count++;
				if (stable_count > 200)
				{
					SYS_INFO("Nose DOWN detected");
					return CalibrationSide::NOSE_DOWN;
				}

				vTaskDelay(10);
				_data = _accel_sub.get();
			}

		}
		// we are on right side
		else if (_data.y > BIG_ENOUGH && !_right_side_calibrated)
		{
			while (_data.y > BIG_ENOUGH)
			{
				stable_count++;
				if (stable_count > 200)
				{
					SYS_INFO("RIGHT side detected");
					return CalibrationSide::RIGHT_SIDE;
				}

				vTaskDelay(10);
				_data = _accel_sub.get();
			}
		}
		// we are on left side
		else if (_data.y < -BIG_ENOUGH && !_left_side_calibrated)
		{
			while (_data.y < -BIG_ENOUGH)
			{
				stable_count++;
				if (stable_count > 200)
				{
					SYS_INFO("LEFT side detected");
					return CalibrationSide::LEFT_SIDE;
				}

				vTaskDelay(10);
				_data = _accel_sub.get();
			}
		}
		// we are upside up
		else if (_data.z > BIG_ENOUGH && !_upside_up_calibrated)
		{
			while (_data.z > BIG_ENOUGH)
			{
				stable_count++;
				if (stable_count > 200)
				{
					SYS_INFO("Upside UP detected");
					return CalibrationSide::UPSIDE_UP;
				}

				vTaskDelay(10);
				_data = _accel_sub.get();
			}
		}
		// we are upside down
		else if (_data.z < -BIG_ENOUGH && !_upside_down_calibrated)
		{
			while (_data.z < -BIG_ENOUGH)
			{
				stable_count++;
				if (stable_count > 200)
				{
					SYS_INFO("Upside DOWN detected");
					return CalibrationSide::UPSIDE_DOWN;
				}

				vTaskDelay(10);
				_data = _accel_sub.get();
			}
		}

		vTaskDelay(1000); // Check for side detection at 1Hz
	}
}

void AccelCalibration::calibrate(CalibrationSide side)
{
	// We will sit for 3 seconds and collect data at 10Hz (accel runs at 1khz)
	auto start = time::HighPrecisionTimer::Instance()->get_absolute_time_us();
	abs_time_t run_time = 3000000; // 3 seconds

	auto now = start;

	_accumulate = 0;
	unsigned num_samples = 0;
	while (now < start + run_time)
	{
		if (_accel_sub.updated())
		{
			_data = _accel_sub.get();

			accumlate_data_for_side(side);
			num_samples++;
		}

		vTaskDelay(100); // run at 10hz -- 30 samples
		now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();
	}

	// update bias for this side
	update_measured_g_for_side(side, num_samples);
}

void AccelCalibration::accumlate_data_for_side(CalibrationSide side)
{
	switch (side)
	{
		case CalibrationSide::UPSIDE_UP:
			_accumulate += _data.z;
			break;

		case CalibrationSide::UPSIDE_DOWN:
			_accumulate += _data.z;
			break;

		case CalibrationSide::LEFT_SIDE:
			_accumulate += _data.y;
			break;

		case CalibrationSide::RIGHT_SIDE:
			_accumulate += _data.y;
			break;

		case CalibrationSide::NOSE_UP:
			_accumulate += _data.x;
			break;

		case CalibrationSide::NOSE_DOWN:
			_accumulate += _data.x;
			break;

		default:
			SYS_INFO("wtf there is no side to calibrate!");
			break;
	}
}

void AccelCalibration::update_measured_g_for_side(CalibrationSide side, unsigned num_samples)
{
	_accumulate /= num_samples;

	switch (side)
	{
		case CalibrationSide::UPSIDE_UP:
			_upside_up_measured_g = _accumulate;
			_upside_up_calibrated = true;
			SYS_INFO("UPSIDE_UP finished: value = %f", _accumulate)
			break;

		case CalibrationSide::UPSIDE_DOWN:
			_upside_down_measured_g = _accumulate;
			_upside_down_calibrated = true;
			SYS_INFO("UPSIDE_DOWN finished: value = %f", _accumulate)
			break;

		case CalibrationSide::LEFT_SIDE:
			_left_side_measured_g = _accumulate;
			_left_side_calibrated = true;
			SYS_INFO("LEFT_SIDE finished: value = %f", _accumulate)
			break;

		case CalibrationSide::RIGHT_SIDE:
			_right_side_measured_g = _accumulate;
			_right_side_calibrated = true;
			SYS_INFO("RIGHT_SIDE finished: value = %f", _accumulate)
			break;

		case CalibrationSide::NOSE_UP:
			_nose_up_measured_g = _accumulate;
			_nose_up_calibrated = true;
			SYS_INFO("NOSE_UP finished: value = %f", _accumulate)
			break;

		case CalibrationSide::NOSE_DOWN:
			_nose_down_measured_g = _accumulate;
			_nose_down_calibrated = true;
			SYS_INFO("NOSE_DOWN finished: value = %f", _accumulate)
			break;

		default:
			SYS_INFO("wtf there is no side to calibrate!");
			break;
	}
}

void AccelCalibration::calculate_offsets_and_scales(void)
{
	// X offset and scale
	_x_offset = (_nose_up_measured_g + _nose_down_measured_g) / 2;
	_x_scale = 2 * GRAVITY_ACCEL / (_nose_up_measured_g - _nose_down_measured_g);

	// Y offset and scale
	_y_offset = (_right_side_measured_g + _left_side_measured_g) / 2;
	_y_scale = 2 * GRAVITY_ACCEL / (_right_side_measured_g - _left_side_measured_g);

	// Z offset and scale
	_z_offset = (_upside_up_measured_g + _upside_down_measured_g) / 2;
	_z_scale = 2 * GRAVITY_ACCEL / (_upside_up_measured_g - _upside_down_measured_g);

	SYS_INFO("x_offset %f", _x_offset)
	SYS_INFO("x_scale %f", _x_scale)

	SYS_INFO("y_offset %f", _y_offset)
	SYS_INFO("y_scale %f", _y_scale)

	SYS_INFO("z_offset %f", _z_offset)
	SYS_INFO("z_scale %f", _z_scale)
}