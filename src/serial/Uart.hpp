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

#pragma once

#include  <board_config.hpp>
#include <functional>

namespace interface
{

static constexpr uint8_t MAX_UARTS = 1;

template <typename T>
class UartBase
{
public:
	~UartBase()
	{
		delete _instance;
	}

	static T* Instantiate(uint8_t uart, unsigned baud, unsigned format)
	{
		if (_instance == nullptr)
		{
			_instance = new T(uart, baud, format);
		}

		return _instance;
	}

	static T* Instance()
	{
		return _instance;
	}

private:
	// friend T;
	static T* _instance;
};

class Uart0 : public UartBase<Uart0>
{
public:

	Uart0(uint8_t uart0_number, unsigned baud, unsigned format) { Serial1.begin(baud, format); };

	~Uart0();

	bool data_available(void);

	uint8_t read(void);

	template <typename T>
	void register_interrupt_callback(T* obj)
	{
		_callback = std::bind(&T::interrupt_callback, obj);
		_callback_registered = true;
	}

	bool interrupt_callback_registered(void) { return _callback_registered; };

	void callback(void) { _callback(); };

private:
	fp_t _callback;
	volatile bool _callback_registered = false;
};

template<typename T>
T* UartBase<T>::_instance = nullptr;

} // end namespace interface
