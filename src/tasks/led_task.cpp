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
#include <Messenger.hpp>
#include <Pwm.hpp>

#define LED_PIN 13

void led_task(void* args)
{
	pinMode(LED_PIN, OUTPUT);

	auto pwm = new Pwm(400);

	for(;;)
	{
		pwm->write(pwm::MOTOR_1, 900);
		pwm->write(pwm::MOTOR_2, 900);
		pwm->write(pwm::MOTOR_3, 900);
		pwm->write(pwm::MOTOR_4, 900);

		digitalWrite(LED_PIN, LOW);

		vTaskDelay(1000);

		pwm->write(pwm::MOTOR_1, 1150);
		pwm->write(pwm::MOTOR_2, 1150);
		pwm->write(pwm::MOTOR_3, 1150);
		pwm->write(pwm::MOTOR_4, 1150);

		digitalWrite(LED_PIN, HIGH);
		vTaskDelay(1000);
	}
}

void sanity_idle_task(void* args)
{
	while(1)
	{
		for (unsigned i = 0;  i < 100000; )
		{
			i++;
		}
	}
}