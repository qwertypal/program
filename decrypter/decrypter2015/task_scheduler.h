#pragma once

//#include <boost\thread\thread.hpp>
//#include <boost\atomic.hpp>

//using namespace boost;
#include <thread>
#include <chrono>
#include <atomic>

#include "crypt.h"

class join_threads
{
private:
	vector<thread>& _threads;
public:
	explicit join_threads(vector<thread> & t) : _threads(t) {}
	~join_threads()
	{
		for(unsigned long i=0; i < _threads.size(); ++i)
		{
			if (_threads[i].joinable())
				_threads[i].join();
		}
	}
};


class Task_scheduler
{
	atomic_bool _stop_threads;
	vector<thread> _threads;
	Task & _task;
	join_threads _joiner;
public:
	Task_scheduler(Task & task) :
	  _task(task),
	  _stop_threads(false),
	  _joiner(_threads)
	{
		const unsigned max_threads = thread::hardware_concurrency();
		cout << "Starting " << max_threads << " concurrent threads." << endl;
		this_thread::sleep_for(chrono::milliseconds(2000));

		for(unsigned i=0; i < max_threads; ++i)
		{
			_threads.push_back(thread(&Task_scheduler::_thread, this));
		}
	}
	~Task_scheduler()
	{
	}

private:
	void _thread()
	{
		while (!_stop_threads)
		{
			try
			{
				if (_task())
					_stop_threads = true;
				else
				{
					this_thread::yield();
				}
			}
			catch (std::exception & e)
			{
				cout << "ERROR: " << e.what() << endl;
				_stop_threads = true;
			}
		}
	}
};

