
#include <kinetis.h>
#include <FreeRTOS.h>
#include <task.h>

#include <Arduino.h>

#define LED_PIN 13U

void LEDTask(void* args)
{
	pinMode(LED_PIN, OUTPUT);

	for(;;)
	{
		digitalWrite(LED_PIN, LOW);
		vTaskDelay(500);
		digitalWrite(LED_PIN, HIGH);
		vTaskDelay(500);
	}
}