// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

template<unsigned Alignment>
class Aligned
{
public:
	static void* operator new(size_t size);
	static void operator delete(void* p);
};

template<unsigned Alignment>
void* Aligned<Alignment>::operator new(size_t size)
{
	return _aligned_malloc(size, Alignment);
}

template<unsigned Alignment>
void  Aligned<Alignment>::operator delete(void* p)
{
	_aligned_free(p);
}
