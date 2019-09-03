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

#include <board_config.hpp>

#define FTM0_MAX_TICKS 6000U // modulo value

static constexpr abs_time_t FTM0_PICOS_PER_TICK = 16667LLU;

extern volatile uint32_t _freertos_stats_base_ticks;

namespace time {

static constexpr abs_time_t MAX_TIME = 0xFFFFFFFFFFFFFFFF;

///////////////////////////////
//---- HIGH PRECISION / DISPATCH SCHEDULING ----//
// Frequency: 60MHz (F_BUS)
// counter: 16bit
// prescaler: 1
// modulo: 6,000 == FTM1_MAX_TICKS
// resolution: 16.6666ns
// overflow rate: 0.1ms --> 10kHz
class HighPrecisionTimer
{
public:

	~HighPrecisionTimer();

	static void Instantiate(void);
	static HighPrecisionTimer* Instance();

	abs_time_t get_absolute_time_us(void);
	abs_time_t get_absolute_time_us_from_isr(void);


	void handle_timer_overflow(void);

	template <typename T>
	void register_overflow_callback(T* obj)
	{
		_callback = std::bind(&T::timer_overflow_callback, obj);
		_callback_registered = true;
	}


private:
	HighPrecisionTimer(){}; // Private so that it can not be called
	HighPrecisionTimer(HighPrecisionTimer const&) {}; // copy constructor is private
	HighPrecisionTimer& operator=(HighPrecisionTimer const&){ return *Instance(); }; // assignment operator is private

	static HighPrecisionTimer* _instance;

	abs_time_t _base_ticks = 0;

	bool _callback_registered = false;
	fp_t _callback; // A function pointer to a callback function that gets called after an overflow event.
};

} // end namespace time
