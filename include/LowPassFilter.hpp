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

template <class T>
class LowPassFilter
{
public:
	LowPassFilter(float cutoff_freq) : _cutoff_freq(cutoff_freq) {}

	float apply(T input, abs_time_t timestamp)
	{
		double dt = (timestamp - _last_timestamp) / (double)MICROS_PER_SEC;
		_last_timestamp = timestamp;

		T RC = 1.0 / (_cutoff_freq * 2 * M_PI);

		T alpha = dt / (RC + dt);

		// Filter the input
		T output = alpha * input + (1 - alpha) * _previous_output;

		_previous_output = output;

		return output;
	}

	void reset(T value)
	{
		_previous_output = value;
	}

private:

    float _cutoff_freq = 0;

    T _previous_output = 0;
    abs_time_t _last_timestamp = 0;
};