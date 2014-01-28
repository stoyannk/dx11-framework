// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include <cstdlib>

class Random
{
public:
	static void Seed(unsigned int seed)
	{
		::srand(seed);
	}

	static float RandomNumber()
	{
		return float(rand()) / RAND_MAX;
	}

	static float RandomBetween(float a, float b)
	{
		return a + RandomNumber()*(b-a);
	}
};