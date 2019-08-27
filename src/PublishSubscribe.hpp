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

#include <list>
#include <string>
#include <atomic>

#include <Time.hpp>

// TODO: autogenerate these...
struct accel_raw_data_s
{
	uint64_t timestamp;
	uint16_t x;
	uint16_t y;
	uint16_t z;
};

struct gyro_raw_data_s
{
	uint64_t timestamp;
	uint16_t x;
	uint16_t y;
	uint16_t z;
};

struct mag_raw_data_s
{
	uint64_t timestamp;
	uint16_t x;
	uint16_t y;
	uint16_t z;
};


// Single publisher multiple subscriber implementation
namespace messenger
{

// A single data file per topic, shared between publisher and subscriber(s)
template <typename T>
class DataFile
{
public:
	T& data(void) { return _data; };

protected:
	static T _data;
};

template<class T>
T DataFile<T>::_data {};

template <typename T>
class Subscriber
{
public:
	bool updated()
	{
		auto data_timestamp = _file->data().timestamp;
		return data_timestamp > _last_timestamp;
	}

	T get()
	{
		auto data = _file->data();
		_last_timestamp = data.timestamp;
		return data;
	};

private:
	// This data is created by publisher and shared with us.
	DataFile<T>* _file;

	uint64_t _last_timestamp = 0; // timestamp from the last data we read.
};

template <typename T>
class Publisher
{
public:
	Publisher()
	{
		// Create the shared data memory
		_file = new DataFile<T>();
	}

	~Publisher()
	{
		// NOTE: we assume the data file is never deleted after being created at init.
		// the consequence of this assumption is that subscribers must always check the timestamp
		// before consuming the data.
	}

	void publish(T& data)
	{
		taskENTER_CRITICAL();
		_file->data() = data;
		taskEXIT_CRITICAL();

		// TODO: notification / scheduling for dependant work.
	}

private:
	DataFile<T>* _file;
};

} // end namespace messenger
