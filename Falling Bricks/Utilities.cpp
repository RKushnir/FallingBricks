#include "StdAfx.h"
#include "Utilities.h"

#include <time.h>

int GetRandomNumber(int nLowerBound, int nUpperBound)
{
	return nLowerBound + rand() % (nUpperBound - nLowerBound);
}

float GetGameTime()
{
	static __int64 nStartTime = 0;
	static __int64 nTimerFrequency = 0;

	if (nStartTime == 0)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&nStartTime);
		QueryPerformanceFrequency((LARGE_INTEGER*)&nTimerFrequency);
		return 0.0f;
	}

	__int64 nCurrentTime = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&nCurrentTime);
	return (float) ((nCurrentTime - nStartTime) / double(nTimerFrequency));
}

