#include "../StdAfx.h"
#include "CmTimer.h"

CmTimer::CmTimer(const string t, const bool warn /* = true */)
: title(t)
, warning(warn)
, is_started(false)
, start_clock(0)
, cumulative_clock(0)
, n_starts(0)
{
}

const CmTimer& CmTimer::operator = (const CmTimer& timer)
{
	memcpy(this, &timer, sizeof(timer));
	return *this;
}

void CmTimer::Start()
{
	if (is_started){
		CmLog::LogLine("CmTimer '%s' is already started. Nothing done.\n", title.c_str());
		return;
	}

	is_started = true;
	n_starts++;
	start_clock = clock();
}

void CmTimer::Stop()
{
	if (!is_started){
		CmLog::LogLine("CmTimer '%s' is started. Nothing done\n", title.c_str());
		return;
	}

	cumulative_clock += clock() - start_clock;
	is_started = false;
}

void CmTimer::Reset()
{
	if (is_started)
	{
		CmLog::LogLine("CmTimer '%s'is started during reset request.\n Only reset cumulative time.\n");
		return;
	}

	cumulative_clock = 0;
}

bool CmTimer::Report()
{
	if (is_started){
		CmLog::LogWarning("CmTimer '%s' is started.\n Cannot provide a time report.", title.c_str());
		return false;
	}

	float timeUsed = TimeInSeconds();

	CmLog::LogLine("[%s] CumuTime: %gs, #run: %d, AvgTime: %gs\n", title.c_str(), timeUsed, n_starts, timeUsed/n_starts);

	return true;
}

float CmTimer::TimeInSeconds()
{
	if (is_started){
		CmLog::LogLine("CmTimer '%s' is started. Nothing done\n", title.c_str());
		return 0;
	}

	return float(cumulative_clock) / CLOCKS_PER_SEC;
}

CmTimer::~CmTimer()
{
	if (is_started)
		CmLog::LogLine("CmTimer '%s' is started and is being destroyed.\n", title.c_str());
}
