#pragma once

class CmTimer
{
public:
	CmTimer(const string t, const bool warn = true);
	~CmTimer();

	const CmTimer& operator = (const CmTimer& timer);

	void Start();
	void Stop();
	void Reset();

	bool Report();

	float TimeInSeconds();

private:
	const string title;
	const bool warning;
	
	bool is_started;
	clock_t start_clock;
	clock_t cumulative_clock;
	unsigned int n_starts;

};
