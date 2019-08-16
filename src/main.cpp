#include <Arduino.h>

// Defining our own main overrides Arduino main (lib/teensy3/main.pp)
// Otherwise we need to define USING_MAKEFILE but that seems dumb
// int main()
// {
// 	// This must be the LED
// 	pinMode(13, OUTPUT);

// 	while (1) 
// 	{
// 		digitalWriteFast(13, HIGH);
// 		delay(100);
// 		digitalWriteFast(13, LOW);
// 		delay(100);
// 	}

// 	return 0;
// }



// As a minimum, the following source files must be included in your project:

// FreeRTOS/Source/tasks.c
// FreeRTOS/Source/queue.c
// FreeRTOS/Source/list.c
// FreeRTOS/Source/portable/[compiler]/[architecture]/port.c.
// FreeRTOS/Source/portable/MemMang/heap_x.c where ‘x’ is 1, 2, 3, 4 or 5.

#include <FreeRTOS.h>
#include <task.h>

// extern tasks declared elsewhere in program
void LEDTask(void* args);
void SerialTask(void* args);

int main() {
  // create the tasks
  xTaskCreate(LEDTask, "LT", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
  xTaskCreate(SerialTask, "ST", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

  // start scheduler, main should stop functioning here
  vTaskStartScheduler();

  for(;;);

  return 0;
}