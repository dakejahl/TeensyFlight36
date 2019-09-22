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
#include <AttitudeControl.hpp>


void controller_task(void* args)
{
	auto attitude_controller = new AttitudeControl();

	for(;;)
	{
		// Get controller command if updated
		attitude_controller->get_rc_input();
		attitude_controller->convert_sticks_to_setpoints();

		attitude_controller->check_for_arm_condition();
		attitude_controller->check_for_kill_condition();

		// Retrieve latest gyro data
		attitude_controller->collect_attitude_data();
		attitude_controller->collect_attitude_rate_data();

		if (attitude_controller->armed())
		{

			attitude_controller->run_controllers();
		}
		else
		{
			attitude_controller->outputs_motors_disarmed();
		}

		// 1kHz loop rate
		vTaskDelay(1);
	}
}
