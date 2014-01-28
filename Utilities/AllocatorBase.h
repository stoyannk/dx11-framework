// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

class AllocatorBase
{
public:
	virtual void* Allocate(size_t sz) = 0;
	virtual void Deallocate(void* ptr) = 0;

	virtual void* AllocateAligned(size_t sz, unsigned alignement) = 0;
	virtual void DeallocateAligned(void* ptr) = 0;
};

template<typename T>
class AllocatorDeleter
{
public:
	AllocatorDeleter()
		: m_Allocator(nullptr)
	{}

	AllocatorDeleter(AllocatorBase* allocator) 
		: m_Allocator(allocator)
	{}

	void operator()(T* ptr) {
		assert(m_Allocator);
		ptr->~T();
		m_Allocator->Deallocate(ptr);
	}

private:
	AllocatorBase* m_Allocator;
};

template<typename T>
class AllocatorSharedDeleter
{
public:
	AllocatorSharedDeleter()
	{}

	AllocatorSharedDeleter(const std::shared_ptr<AllocatorBase>& allocator) 
		: m_Allocator(allocator)
	{}

	void operator()(T* ptr) {
		assert(m_Allocator);
		ptr->~T();
		m_Allocator->Deallocate(ptr);
	}

private:
	std::shared_ptr<AllocatorBase> m_Allocator;
};

template<typename T>
class AllocatorAlignedDeleter
{
public:
	AllocatorAlignedDeleter()
		: m_Allocator(nullptr)
	{}

	AllocatorAlignedDeleter(AllocatorBase* allocator) 
		: m_Allocator(allocator)
	{}

	void operator()(T* ptr) {
		assert(m_Allocator);
		ptr->~T();
		m_Allocator->DeallocateAligned(ptr);
	}

private:
	AllocatorBase* m_Allocator;
};