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

#include <timers/Time.hpp>

extern volatile uint32_t _freertos_stats_base_ticks = 0;

namespace time {

// Global static pointer used to ensure a single instance of the class.
PrecisionTimer* PrecisionTimer::_instance = nullptr;
DispatchTimer* DispatchTimer::_instance = nullptr;

void PrecisionTimer::Instantiate(void)
{
	if (!_instance)
	{
		_instance = new PrecisionTimer();
	}
}

PrecisionTimer* PrecisionTimer::Instance()
{
	return _instance;
}

PrecisionTimer::~PrecisionTimer()
{
	delete _instance;
}

abs_time_t PrecisionTimer::get_absolute_time_us(void)
{
	abs_time_t current_time;
	uint16_t tick_val;

	// Not reentrant
	taskENTER_CRITICAL();

	tick_val = FTM0_CNT;

	current_time = FTM0_MICROS_PER_TICK * (_base_ticks + tick_val);

	taskEXIT_CRITICAL();

	return current_time;
}

uint64_t PrecisionTimer::get_ticks_since_boot(void)
{
	uint16_t tick_val;
	uint64_t total_ticks;

	taskENTER_CRITICAL();

	tick_val = FTM0_CNT;

	total_ticks = _base_ticks + tick_val;

	taskEXIT_CRITICAL();

	return total_ticks;
}

void PrecisionTimer::handle_timer_overflow(void)
{
	_base_ticks += FTM0_MAX_TICKS;
	_freertos_stats_base_ticks = _base_ticks;
}

//---- DISPATCH TIMER ----//
DispatchTimer::DispatchTimer(DispatchQueue* queue)
	: _dispatch_queue(queue)
{
	// Nothing to do
}
void DispatchTimer::Instantiate(DispatchQueue* queue)
{
	if (!_instance)
	{
		_instance = new DispatchTimer(queue);
	}
}

DispatchTimer* DispatchTimer::Instance()
{
	return _instance;
}

DispatchTimer::~DispatchTimer()
{
	delete _instance;
}

abs_time_t DispatchTimer::get_absolute_time_ms(void)
{
	// Not reentrant
	taskENTER_CRITICAL();

	abs_time_t current_time = _base_ticks;

	taskEXIT_CRITICAL();

	return current_time;
}

void DispatchTimer::handle_timer_overflow(void)
{
	// Overflows every millisecond so we just use this as our counter
	_base_ticks += 1U;

	// Schedule an item if it's ready
	abs_time_t current_time_ms = _base_ticks;
	// 1 tick equals 16.6666ns
	if (current_time_ms > _next_deadline_ms)
	{
		if (_dispatch_queue != nullptr)
		{
			_dispatch_queue->interval_dispatch_notify_ready();
		}
	}
}

// MUST ONLY BE CALLED WITTH INTERRUPTS DISABLED
void DispatchTimer::set_next_deadline_ms(abs_time_t deadline_ms)
{
	_next_deadline_ms = deadline_ms;
}

} // end namespace time

// Define ISR for FlexTimer Module 0
extern "C" void ftm0_isr(void)
{
	auto saved_state = taskENTER_CRITICAL_FROM_ISR();

	{
		// We've overflown -- add the full scale value to the base
		time::PrecisionTimer::Instance()->handle_timer_overflow();
	}

	// Clear overflow flag -- reset happens on overflow (FTM0_MOD = 0xFFFF)
	if ((FTM0_SC & FTM_SC_TOF) != 0)
	{
		FTM0_SC &= ~FTM_SC_TOF;
		FTM0_CNT = 0;
	}

	taskEXIT_CRITICAL_FROM_ISR(saved_state);
}

// Define ISR for FlexTimer Module 1
extern "C" void ftm1_isr(void)
{
	auto saved_state = taskENTER_CRITICAL_FROM_ISR();

	{
		if (time::DispatchTimer::Instance() != nullptr)
		{
			time::DispatchTimer::Instance()->handle_timer_overflow();
		}
	}

	// Clear overflow flag -- reset happens on overflow (FTM0_MOD = 0xFFFF)
	if ((FTM1_SC & FTM_SC_TOF) != 0)
	{
		FTM1_SC &= ~FTM_SC_TOF;
		FTM1_CNT = 0;
	}

	taskEXIT_CRITICAL_FROM_ISR(saved_state);
}