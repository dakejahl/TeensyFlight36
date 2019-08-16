#include <FreeRTOS.h>
#include <task.h>

#include <Arduino.h>
#include <usb_dev.h>



void SerialTask(void* args)
{

	// pinMode(13, OUTPUT);

	// initialize the serial
	usb_init();
	Serial.begin(9600);

	for (;;)
	{
		// digitalWrite(13, LOW);
		// testDelayMS(500);
		// digitalWrite(13, HIGH);
		Serial.println("Hello World");
		if (Serial.available())
		{
			serialEvent();
		} 
		
		vTaskDelay(500);
	}
}
