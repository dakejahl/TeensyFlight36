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

#include <Arduino.h>

#include <FreeRTOS.h>
#include <task.h>

#include <board_config.hpp>
#include <dispatch_queue/DispatchQueue.hpp>

// Let's test our assumptions.
// All of these should hold true, otherwise compilation fails.
static_assert(F_CPU == 180000000, "F_CPU is not 180MHz");
static_assert(configTICK_RATE_HZ == 1000, "SYSTICK is not 1KHz");


// printf convenience macro
// TODO: make it thread safe
#define _PRINTF_BUFFER_LENGTH_ 64U
static char _pf_buffer_[_PRINTF_BUFFER_LENGTH_];
#define printf(a,...)														\
	do{																		\
	snprintf(_pf_buffer_, sizeof(_pf_buffer_), a, ##__VA_ARGS__);			\
	Serial.print(_pf_buffer_);												\
	}while(0);

extern void LEDTask(void* args);
extern void DummyTask(void* args);





int main()
{
	// Create the tasks
	// xTaskCreate(LEDTask, "LT", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	xTaskCreate(DummyTask, "DT", configMINIMAL_STACK_SIZE, NULL, 0, NULL);

	// Serial.begin(9600);

	vTaskStartScheduler();

	for(;;);

	return 0;
}

volatile extern bool _isr_flag_;
void DummyTask(void* args)
{
	#define LED_PIN 13
	pinMode(LED_PIN, OUTPUT);

	digitalWrite(LED_PIN, LOW);
	bool led_on = false;

	unsigned counter = 0;
	while(1)
	{
		// Count ISR signals
		if (_isr_flag_)
		{
			counter++;
			_isr_flag_ = false;
		}

		// Turn LED on after a period
		if (counter == 100 && led_on)
		{
			digitalWrite(LED_PIN, LOW);
			led_on = false;
			counter = 0;
		}
		// Turn LED off after a period
		else if (counter == 100 && !led_on)
		{
			digitalWrite(LED_PIN, HIGH);
			led_on = true;
			counter = 0;
		}
	}
}

// The ONLY place Serial.* gets used
extern "C" void vApplicationIdleHook(void)
{
	// Add profiling here
	// Add shell here
	// -- printing
	// -- reading --> executing --> DispatchQueue!
}
