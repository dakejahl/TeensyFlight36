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

#include <vector>
#include <string>
#include <atomic>

#include <timers/Time.hpp>

// TODO: autogenerate these...
struct __attribute__((__packed__)) accel_raw_data_s
{
	abs_time_t timestamp;
	float temperature;
	float x;
	float y;
	float z;
};

struct __attribute__((__packed__)) gyro_raw_data_s
{
	abs_time_t timestamp;
	float temperature;
	float x;
	float y;
	float z;
};

struct __attribute__((__packed__)) gyro_filtered_data_s
{
	abs_time_t timestamp;
	float temperature;
	float x;
	float y;
	float z;
};

struct __attribute__((__packed__)) mag_raw_data_s
{
	abs_time_t timestamp;
	float temperature;
	float x;
	float y;
	float z;
};

struct __attribute__((__packed__)) rc_input_s
{
	abs_time_t timestamp;
	uint16_t channels[16];
	uint16_t lost_frame_count;
	bool rc_failsafe;
	bool rc_lost;
};

struct __attribute__((__packed__)) attitude_euler
{
	abs_time_t timestamp;
	float roll;
	float pitch;
	float yaw;
};

// Multiple publisher multiple subscriber implementation
// with a (SHARED!) data file.
namespace messenger
{

template <typename T>
class Subscriber;

// Static class!
template <typename T>
class DataFile
{
public:
	static T& get_data(void)
	{
		return _data;
	};

	static void set_data(T data) { _data = data; };
	static void add_subscriber(Subscriber<T>* sub)
	{
		_subscribers.push_back(sub);
	}

	static void notify_subscribers(void)
	{
		for (auto& s : _subscribers)
		{
			s->notify();
		}
	}

protected:
	static T _data; // static container for the data
	static std::vector<Subscriber<T>*> _subscribers;
};
template<class T>
T DataFile<T>::_data {};
template<class T>
std::vector<Subscriber<T>*> DataFile<T>::_subscribers;

template <typename T>
class Subscriber
{
public:
	Subscriber()
	{
		taskENTER_CRITICAL();

		_file->add_subscriber(this);

		taskEXIT_CRITICAL();
	}

	void notify(void)
	{
		_updated = true;
	}

	// Dumb impl -- subscribers must poll
	bool updated(void)
	{
		return _updated;
	}

	T get(void)
	{
		taskENTER_CRITICAL();

		auto data = _file->get_data();
		_updated = false;

		taskEXIT_CRITICAL();

		return data;
	};

private:
	bool _updated = false; // TODO: std::atomic
	DataFile<T>* _file;
};

template <typename T>
class Publisher
{
public:
	void publish(T& data)
	{
		taskENTER_CRITICAL();

		_file->set_data(data);
		_file->notify_subscribers();

		taskEXIT_CRITICAL();

		// TODO: notification / scheduling for dependent work.
	}
private:
	DataFile<T>* _file;
};

} // end namespace messenger
