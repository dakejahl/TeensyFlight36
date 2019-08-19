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

static void init_FTM0(void);

extern "C" void sys_init(void)
{
	// Set up custom peripherals
	init_FTM0();
}

#define FTM_PRESCALE_533_NANO_SEC 32
#define FTM_SYS_CLK 1
// Frequency: 60MHz (F_BUS)
// counter: 16bit
// prescaler: 32
// resolution: 0.533us
// overflow rate: 34.952ms --> 28.61Hz
static void init_FTM0(void)
{
	// Disable write protection to change the settings -- TODO reenable write protection?
	FTM0_MODE = FTM_MODE_WPDIS | FTM_MODE_FTMEN;
	FTM0_CNT = 0x0000; //reset count to zero
	FTM0_MOD = 0xFFFF; //max modulus = 65535 (gives count = 65,536 on roll-over)
	// Turn the timer on and configure with our settings
	FTM0_SC = FTM_SC_CLKS(FTM_SYS_CLK) // Set to system clock
			| FTM_SC_PS(64) // set prescaler for desired resolution / overflow rate
			| FTM_SC_TOIE; // enable overflow interrupt

	// Enable the interrupt
	NVIC_ENABLE_IRQ(IRQ_FTM0);
}

volatile extern bool _isr_flag_ = 0;
volatile unsigned toggle = 0;
extern "C" void ftm0_isr(void)
{
	{ // Implement custom code?
		_isr_flag_ = true;
	}

	// Clear overflow flag -- reset happens on overflow (FTM0_MOD = 0xFFFF)
	if ((FTM0_SC & FTM_SC_TOF) != 0)
	{
		FTM0_SC &= ~FTM_SC_TOF;
	}
}
