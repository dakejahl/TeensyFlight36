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
#include <dispatch_queue/DispatchQueue.hpp>

#define FTM0_MAX_TICKS 65535U // modulo value
#define FTM1_MAX_TICKS 6000U // modulo value

extern volatile uint32_t _freertos_stats_base_ticks;

namespace time {

static constexpr abs_time_t MAX_TIME = 0xFFFFFFFFFFFFFFFF;
///////////////////////////////
// ------ Specifications ------
// Resolution: 0.533us
// Overflow rate: 28.61Hz --> 34.952ms
class PrecisionTimer
{
public:

	~PrecisionTimer();

	static void Instantiate(void);
	static PrecisionTimer* Instance();

	abs_time_t get_absolute_time_us(void);

	void handle_timer_overflow(void);

private:
	PrecisionTimer(){}; // Private so that it can  not be called
	PrecisionTimer(PrecisionTimer const&){}; // copy constructor is private
	PrecisionTimer& operator=(PrecisionTimer const&){return *Instance();}; // assignment operator is private

	static PrecisionTimer* _instance;

	abs_time_t _base_ticks = 0;
};

// TODO: WARNING: since this overflows at 1kHz it means we have up to +/- 1ms of jitter
// This timer is used to schedule items onto the DispatchQueue
// The timer shall have a resolution of 1ms and be of high priority
class DispatchTimer
{
public:
	DispatchTimer(DispatchQueue* queue);
	~DispatchTimer();

	abs_time_t get_absolute_time_us(void);

	static void Instantiate(DispatchQueue* queue);
	static DispatchTimer* Instance();

	void handle_timer_overflow(void);
	void schedule_next_deadline_us(abs_time_t deadline_us);
	void disable_scheduling(void);

private:
	DispatchTimer(){}; // Private so that it can  not be called
	DispatchTimer(DispatchTimer const&){}; // copy constructor is private
	DispatchTimer& operator=(DispatchTimer const&){return *Instance();}; // assignment operator is private

	static DispatchTimer* _instance;

	DispatchQueue* _dispatch_queue = nullptr;

	abs_time_t _base_ticks = 0;
	abs_time_t _next_deadline_us = MAX_TIME;
};

} // end namespace time
