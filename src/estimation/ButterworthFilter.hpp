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

#include <cmath>
// Second order butterworth filter

class ButterworthFilter
{
public:

	ButterworthFilter(float sample_freq, float cutoff_freq)
	{
		// set initial parameters
		set_cutoff_frequency(sample_freq, cutoff_freq);
	}

	// Change filter parameters
	void set_cutoff_frequency(float sample_freq, float cutoff_freq);

	/**
	 * Add a new raw value to the filter
	 *
	 * @return retrieve the filtered result
	 */
	float apply(float sample);

	// Return the cutoff frequency
	float get_cutoff_freq() const { return _cutoff_freq; }

	// Reset the filter state to this value
	float reset(float sample);

private:

	float _cutoff_freq{0.0f};

	float _a1{0.0f};
	float _a2{0.0f};

	float _b0{0.0f};
	float _b1{0.0f};
	float _b2{0.0f};

	float _delay_element_1{0.0f};	// buffered sample -1
	float _delay_element_2{0.0f};	// buffered sample -2
};
