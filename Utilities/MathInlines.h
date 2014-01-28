// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

namespace StMath
{
// Faster versions of min/max due to a VS bug explainfed here:
// http://randomascii.wordpress.com/2013/11/24/stdmin-causing-three-times-slowdown-on-vc/
template <class T>
T max_value(const T& left, const T& right)
{
	return left > right ? left : right;
}

template <class T>
T min_value(const T& left, const T& right)
{
	return left < right ? left : right;
}

template<typename T>
T clamp_value(T value, T min, T max)
{
	return min_value(max, max_value(min, value));
}

}