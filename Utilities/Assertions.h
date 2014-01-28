// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include <assert.h>

#define Assert(EXP, RETURN) \
	assert(EXP); \
	if(!EXP) \
	{ \
		return RETURN; \
	} \

#define ReturnIf(X, ...) \
	if(X) \
	{ \
		return __VA_ARGS__; \
	} \

#define ReturnUnless(X, ...) ReturnIf(!X, __VA_ARGS__)
