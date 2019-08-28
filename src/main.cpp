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
#include <Time.hpp>

// Let's test our assumptions.
// All of these should hold true, otherwise compilation fails.
static_assert(F_CPU == 180000000, "F_CPU is not 180MHz");
static_assert(configTICK_RATE_HZ == 1000, "SYSTICK is not 1KHz");


extern void led_task(void* args);
extern void talker_task(void* args);
extern void listener_task(void* args);
extern void dispatch_interval_task(void* args);

extern time::PrecisionTimer* PrecisionTimer;
extern const uint8_t FreeRTOSDebugConfig[];

extern "C" int main()
{
	// FreeRTOSDebugConfig[0] = 0xFF;
	// SHOULD NEVER BE ZERO!!!
	if (FreeRTOSDebugConfig[0] == 0)
	{ /* just use it, so the linker cannot remove FreeRTOSDebugConfig[] */
		for(;;); /* FreeRTOSDebugConfig[0] should always be non-zero, so this should never happen */
	}

	// Init system timer
	time::PrecisionTimer::Instantiate();

	// LED task that tells us all is OK
	xTaskCreate(led_task, "led_task", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	xTaskCreate(talker_task, "talker", configMINIMAL_STACK_SIZE * 2, NULL, 3, NULL);
	xTaskCreate(listener_task, "listener", configMINIMAL_STACK_SIZE * 3, NULL, 3, NULL);
	xTaskCreate(dispatch_interval_task, "dispatch_interval", configMINIMAL_STACK_SIZE * 5, NULL, 3, NULL);

	vTaskStartScheduler();

	for(;;);

	return 0;
}

// The ONLY place Serial.* gets used
extern "C" void vApplicationIdleHook(void)
{
	// Add shell here
}
