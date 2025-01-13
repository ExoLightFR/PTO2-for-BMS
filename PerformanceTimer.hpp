#pragma once

#ifndef NOMINMAX
# define NOMINMAX
#endif

#include <windows.h>

#ifdef NOMINMAX
# undef NOMINMAX
#endif

// https://stackoverflow.com/a/483364
class PerformanceTimer
{
	LARGE_INTEGER start_time_;
public:
	PerformanceTimer() { QueryPerformanceCounter(&start_time_); }
	void   restart() { QueryPerformanceCounter(&start_time_); }
	double elapsed() const
	{
		LARGE_INTEGER end_time, frequency;
		QueryPerformanceCounter(&end_time);
		QueryPerformanceFrequency(&frequency);
		return double(end_time.QuadPart - start_time_.QuadPart)
			/ frequency.QuadPart;
	}
};

template< typename Func >
double measure_time(Func f)
{
	PerformanceTimer t;
	f();
	return t.elapsed();
}
