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

#include <functional>

#include <Arduino.h>
#include "core_pins.h"
#include <kinetis.h>

#include <FreeRTOS.h>
#include <task.h>

#include <spi4teensy3.hpp>

#define BOUNCE(c,m) bounce<c, decltype(&c::m), &c::m>

// Bounce for C++ --> C function callbacks
template<class T, class Method, Method m, class ...Params>
static auto bounce(void *priv, Params... params) ->
		decltype(((*reinterpret_cast<T *>(priv)).*m)(params...))
{
	return ((*reinterpret_cast<T *>(priv)).*m)(params...);
}

typedef std::function<void(void)> fp_t;

// Helpers for understanding my config
#define STR_HELPER(x) #x
#define TO_STRING(x) STR_HELPER(x)

using abs_time_t = uint64_t;


static constexpr abs_time_t PICOS_PER_MICRO = 1000000LLU;
static constexpr abs_time_t MICROS_PER_MILLI = 1000LLU;

// Setup peripherals etc
extern "C" void SystemInit(void);

#define _PRINTF_BUFFER_LENGTH_ 64U
#define SYS_PRINT() Serial.println(_pf_buffer_)
#define SYS_INFO(fmt,...)                                               \
do{                                                                     \
	char _pf_buffer_[_PRINTF_BUFFER_LENGTH_];							\
	snprintf(_pf_buffer_, sizeof(_pf_buffer_), fmt, ##__VA_ARGS__);		\
	SYS_PRINT();                                                        \
	}while(0);

enum PriorityLevel : uint8_t
{
	LOWEST = 0,
	HIGHEST = configMAX_PRIORITIES,
};