#include <Arduino.h>

// Defining our own main overrides Arduino main (lib/teensy3/main.pp)
// Otherwise we need to define USING_MAKEFILE but that seems dumb
int main()
{
	// This must be the LED
	pinMode(13, OUTPUT);

	while (1) 
	{
		digitalWriteFast(13, HIGH);
		delay(1000);
		digitalWriteFast(13, LOW);
		delay(1000);
	}

	return 0;
}

