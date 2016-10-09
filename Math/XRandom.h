
#pragma once
#include <stdlib.h>
#include "math.h"
#include <time.h>

namespace Math
{
	#define		LERP_TIME		1000
	#define		RANDOM_SEED		1000
	#define		FLOAT_DIFF		0.0001f

	inline void XRandomSeedInit()
	{
		srand((unsigned int)time(NULL));
	}

	inline uint32 XRandom(uint32 nMaxValue)
	{
		return /*1 + */(uint32)((float)nMaxValue * rand() / (RAND_MAX + 1.0f));
	}

	inline uint32 XRandom(uint32 nMinValue, uint32 nMaxValue)
	{
		int nTemp = 0;

		if (nMinValue > nMaxValue)
		{
			nTemp = nMinValue;
			nMinValue = nMaxValue;
			nMaxValue = nTemp;
		}
		return nMinValue + /*1 + */(uint32)((float)(nMaxValue - nMinValue)* rand() / (RAND_MAX + 1.0f));
	}

	//
	// from nMinValue to nMaxValue
	//
	inline int XRandom(int nMinValue, int nMaxValue)
	{
		if (nMaxValue > nMinValue)
			return nMinValue + (nMaxValue - nMinValue) * rand() / RAND_MAX;
		else
			return nMaxValue + (nMinValue - nMaxValue) * rand() / RAND_MAX;
	}

	inline float XRandom(float MinValue, float MaxValue)
	{
		float Temp = 0.0f;

		if (MinValue > MaxValue)
		{
			Temp = MinValue;
			MinValue = MaxValue;
			MaxValue = Temp;
		}
		return MinValue + (MaxValue - MinValue)*rand() / RAND_MAX;
	}

	inline float XLerpRandom(float fRange, float fCycle, float fPosX, float fPhase)
	{
		float fResult = 0.00f;
		static int nLoopCount = 0;

		static float _fRange = 0.0f;
		static float _fCycle = 0.0f;
		static float _fPosX = 0.0f;
		static float _fPhase = 0.0f;

		if (fabs(fRange - _fRange) < FLOAT_DIFF ||
			fabs(fCycle - _fCycle) < FLOAT_DIFF ||
			fabs(fPosX - _fPosX) < FLOAT_DIFF ||
			fabs(fPhase - _fPhase) < FLOAT_DIFF)
		{
			_fRange = fRange;
			_fCycle = fCycle;
			_fPosX = fPosX;
			_fPhase = fPhase;
			nLoopCount = 0;
		}

		if (nLoopCount > LERP_TIME)
			nLoopCount = 0;
		else
			nLoopCount++;

		//fResult = fRange * sin(0.1f * (fCycle + (float)(nLoopCount / LERP_TIME)) * fPosX + fPhase);
		for (int i = 0; i < LERP_TIME; i++)
		{
			fResult += fRange * sin(0.1f * (fCycle + (float)(nLoopCount / LERP_TIME)) * fPosX + fPhase);
		}

		return fResult / LERP_TIME;
	}
}
using namespace Math;

