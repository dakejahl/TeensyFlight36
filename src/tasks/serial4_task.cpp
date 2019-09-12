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


void serial4_task(void* arg)
{
	Serial4.begin(9600, SERIAL_8N1);
	Serial4.println("x,y,z");

	messenger::Subscriber<mag_raw_data_s> mag_sub;


	for(;;)
	{
		auto data = mag_sub.get();
		float x = data.x;
		float y = data.y;
		float z = data.z;

		// Serial4.printf("%f,%f,%f\n", x, y, z);

		Serial4.print(x);
		Serial4.print(',');
		Serial4.print(y);
		Serial4.print(',');
		Serial4.print(z);
		Serial4.print("\n");

		// 25 bytes here...
		// 12 in data...
		//

		vTaskDelay(10);
	}
}
