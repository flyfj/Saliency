#pragma once

#include <time.h>
#include <string>
using namespace std;

// example usage
/*
tic();
for(...)
{
do_something();
double seconds;
toc(seconds, false);
printf("seconds : %f", seconds);
}
toc("total time : ");
*/

void tic();
string toc(const char* tip = "", bool over = true);
bool toc(int& mins, double& seconds, bool over = true);
bool toc(double& seconds, bool over = true);

// example usage
/*
Timer timer;
step1();
float flDeltaLast, flDeltaStart;
timer.Stamp(flDeltaLast, flDeltaStart);
printf("time from start : %fs, time in step 1: %fs", flDeltaLast, flDeltaStart);

step2();
timer.Stamp(flDeltaLast, flDeltaStart);
printf("time from start : %fs, time in step 2: %fs", flDeltaLast, flDeltaStart);
*/
#include <Windows.h>
class Timer
{
public:
	explicit Timer()
	{
		QueryPerformanceFrequency(&m_iFrequency);
		QueryPerformanceCounter(&m_iLast);
		m_iStart = m_iLast;
	}

	float Stamp()
	{
		LARGE_INTEGER iNow = {0};
		QueryPerformanceCounter(&iNow);
		return float(iNow.QuadPart - m_iStart.QuadPart) / m_iFrequency.QuadPart;
	}

	void Stamp(float& flDeltaLast, float& flDeltaStart)
	{
		LARGE_INTEGER iNow = {0};
		QueryPerformanceCounter(&iNow);
		flDeltaLast = float(iNow.QuadPart - m_iLast.QuadPart) / m_iFrequency.QuadPart;
		flDeltaStart = float(iNow.QuadPart - m_iStart.QuadPart) / m_iFrequency.QuadPart;
		m_iLast = iNow;
	}

protected:
	LARGE_INTEGER m_iStart;
	LARGE_INTEGER m_iLast;
	LARGE_INTEGER m_iFrequency;
};
