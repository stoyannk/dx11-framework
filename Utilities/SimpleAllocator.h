// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "AllocatorBase.h"

class SimpleAllocator : public AllocatorBase
{
	virtual void* Allocate(size_t sz) override;
	virtual void Deallocate(void* ptr) override;

	virtual void* AllocateAligned(size_t sz, unsigned alignement) override;
	virtual void DeallocateAligned(void* ptr) override;
};