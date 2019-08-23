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

using abs_time_t = uint64_t;
#define FTM_MAX_TICKS 65536

extern volatile uint32_t _freertos_stats_base_ticks;

namespace time {

///////////////////////////////
// ------ Specifications ------
// Resolution: 0.533us
// Overflow rate: 28.61Hz --> 34.952ms
class SystemTimer
{
public:

	~SystemTimer();

	static void Instantiate(void);
	static SystemTimer* Instance();

	abs_time_t get_absolute_time_us(void);

	abs_time_t get_ticks_since_boot(void);

	void handle_timer_overflow(void);

private:
	SystemTimer(){}; // Private so that it can  not be called
	SystemTimer(SystemTimer const&){}; // copy constructor is private
	SystemTimer& operator=(SystemTimer const&){return *Instance();}; // assignment operator is private

	static SystemTimer* _instance;

	abs_time_t _base_ticks = 0;
};

} // end namespace time
