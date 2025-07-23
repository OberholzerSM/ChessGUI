#pragma once
#include <chrono>

class Timer
{
public:
	Timer()
	{
		start_time = clock::now();
	}

	void reset()
	{
		start_time = clock::now();
	}

	void pause()
	{
		pause_time = clock::now();
		paused = true;
	}

	void resume()
	{
		if (paused)
			start_time += clock::now() - pause_time;
		paused = false;
	}

	double getTime() const
	{
		if (paused)
			return std::chrono::duration_cast<double_second>(pause_time - start_time).count();
		else
			return std::chrono::duration_cast<double_second>(clock::now() - start_time).count();
	}

private:
	using clock = std::chrono::steady_clock;
	using time_point = std::chrono::time_point<clock>;
	using double_second = std::chrono::duration<double, std::ratio<1> >;  //std::ratio<1> repräsentiert Sekunden, 1:1000 Millisekunden etc.
	time_point start_time, pause_time;
	bool paused{false};
};