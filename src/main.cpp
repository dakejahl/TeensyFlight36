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
#include <timers/Time.hpp>

extern void led_task(void* args);
extern void serial_uart_task(void* args);
extern void estimator_task(void* args);
extern void imu_task(void* args);
extern void sanity_idle_task(void* args);
extern void dispatch_test_task(void* args);
extern void shell_task(void* args);


extern const uint8_t FreeRTOSDebugConfig[];

extern "C" int main()
{
	// Used for time keeping and interval dispatching
	time::HighPrecisionTimer::Instantiate();

	// Initialize SystemView
	SEGGER_SYSVIEW_Conf();

	// SystemView will mark unintrumented work as "idle", which is very misleading! Keep this in here.
	xTaskCreate(sanity_idle_task, "sanity_idle_task", configMINIMAL_STACK_SIZE, NULL, PriorityLevel::LOWEST, NULL);
	xTaskCreate(shell_task, "shell_task", configMINIMAL_STACK_SIZE*4, NULL, PriorityLevel::LOWEST+1, NULL);

	xTaskCreate(led_task, "led_task", configMINIMAL_STACK_SIZE * 3, NULL, PriorityLevel::LOWEST+1, NULL);
	xTaskCreate(estimator_task, "estimator", configMINIMAL_STACK_SIZE * 3, NULL, PriorityLevel::LOWEST+1, NULL);

	xTaskCreate(serial_uart_task, "frsky", configMINIMAL_STACK_SIZE * 4, NULL, PriorityLevel::LOWEST+2, NULL);

	xTaskCreate(dispatch_test_task, "dispatch_test_task", configMINIMAL_STACK_SIZE * 5, NULL, PriorityLevel::HIGHEST, NULL);
	xTaskCreate(imu_task, "imu_task", configMINIMAL_STACK_SIZE * 5, NULL, PriorityLevel::HIGHEST-1, NULL);

	vTaskStartScheduler();

	for(;;);

	return 0;
}

extern "C" void vApplicationIdleHook(void)
{
	// Add shell here
}
