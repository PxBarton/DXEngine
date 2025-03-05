#pragma once

#include <chrono>

class Timer
{
public:
	Timer()
	{
		start = std::chrono::high_resolution_clock::now();
		stop = std::chrono::high_resolution_clock::now();
	}
	double GetMilisecondsElapsed()
	{
		if (isrunning)
		{
			auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start);
			return elapsed.count();
		}
		else
		{
			auto elapsed = std::chrono::duration<double, std::milli>(stop - start);
			return elapsed.count();
		}
	}

	void Restart()
	{
		isrunning = true;
		start = std::chrono::high_resolution_clock::now();
	}

	bool Stop()
	{
		if (!isrunning)
			return false;
		else
		{
			stop = std::chrono::high_resolution_clock::now();
			isrunning = false;
			return true;
		}
	}

	bool Start()
	{
		if (isrunning)
		{
			return false;
		}
		else
		{
			start = std::chrono::high_resolution_clock::now();
			isrunning = true;
			return true;
		}
	}

private:
	bool isrunning = false;

#ifdef _WIN32
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::time_point<std::chrono::steady_clock> stop;
#else
	std::chrono::time_point<std::chrono::system_clock> start;
	std::chrono::time_point<std::chrono::system_clock> stop;
#endif
};


