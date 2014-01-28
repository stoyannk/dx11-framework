// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

class CodeTimer // NB: use the class only from threads with proper affinity mask set
{
public:
	CodeTimer()
	{
		LARGE_INTEGER ticks;
		::QueryPerformanceCounter(&ticks);
		m_StartTime = ticks.QuadPart;
	}

	double Stop() const
	{
		LARGE_INTEGER ticks;
		::QueryPerformanceCounter(&ticks);

		return (ticks.QuadPart - m_StartTime) / double(GetFrequency());
	}

	static long long GetFrequency() // if used by differrent threads that work on different freq cores this won't work
	{
		static long long freq = 0;
		if(freq == 0)
		{
			LARGE_INTEGER f;
			QueryPerformanceFrequency(&f);
			freq = f.QuadPart;
		}
		return freq;
	}

private:
	long long m_StartTime;
};