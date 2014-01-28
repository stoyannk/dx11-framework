// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "SimpleAllocator.h"

void* SimpleAllocator::Allocate(size_t sz)
{
	return ::malloc(sz);
}

void SimpleAllocator::Deallocate(void* ptr)
{
	::free(ptr);
}

void* SimpleAllocator::AllocateAligned(size_t sz, unsigned alignement)
{
	return _aligned_malloc(sz, alignement);
}

void SimpleAllocator::DeallocateAligned(void* ptr)
{
	return _aligned_free(ptr);
}
