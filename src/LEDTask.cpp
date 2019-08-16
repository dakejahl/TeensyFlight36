
#include <kinetis.h>

#include <FreeRTOS.h>
#include <task.h>

int LED_CONFIG = 0x00000102;
int LED_MASK   = 0x00000020;
int counter = 0;

#include <Arduino.h>

int ledPin = 13;

/** calibration factor for delayMS */
#define CAL_FACTOR (F_CPU/6007)
/** delay between led error flashes
 * \param[in] millis milliseconds to delay
 */
void testDelayMS(uint32_t millis) {
  uint32_t iterations = millis * CAL_FACTOR;
  uint32_t i;
  for(i = 0; i < iterations; ++i) {
    asm volatile("nop\n\t");
  }
}

void LEDTask(void* args) {
  pinMode(ledPin, OUTPUT);

  for(;;)
  {

    digitalWrite(ledPin, LOW);
    vTaskDelay(500);
    digitalWrite(ledPin, HIGH);
    vTaskDelay(500);
  }
}

/* pure ARM configuration
  // setup
  PORTC_PCR5 = LED_CONFIG;
  GPIOC_PDDR |= LED_MASK;

  // execution
  for(;;) {
    if (counter++ % 2) {
      GPIOC_PDOR |= LED_MASK;
    } else {
      GPIOC_PDOR &= ~LED_MASK;
    }
  }
*/
