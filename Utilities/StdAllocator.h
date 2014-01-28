// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include <Utilities/AllocatorBase.h>

template<typename T>
class StdAllocator {
public : 
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

public : 
    template<typename U>
    struct rebind {
        typedef StdAllocator<U> other;
    };

public : 
    inline explicit StdAllocator(const std::shared_ptr<AllocatorBase>& allocator)
		: m_Allocator(allocator)
	{}
    
	inline ~StdAllocator() {}
    inline StdAllocator(const StdAllocator& rhs)
		: m_Allocator(rhs.m_Allocator)
	{}

    template<typename U>
    inline StdAllocator(const StdAllocator<U>& rhs)
		: m_Allocator(rhs.m_Allocator)
	{}

    inline pointer address(reference r) { return &r; }
    inline const_pointer address(const_reference r) { return &r; }

    inline pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer = 0) 
	{ 
		return reinterpret_cast<pointer>(m_Allocator->Allocate(cnt * sizeof (T))); 
    }
    inline void deallocate(pointer p, size_type) 
	{
		m_Allocator->Deallocate(p);
    }

    inline size_type max_size() const 
	{ 
        return std::numeric_limits<size_type>::max() / sizeof(T);
	}

    inline void construct(pointer p, const T& t) { new(p) T(t); }
    inline void destroy(pointer p) { p->~T(); }

    inline bool operator==(StdAllocator const&) { return true; }
    inline bool operator!=(StdAllocator const& a) { return !operator==(a); }

private:
	template<typename U> friend class StdAllocator;
	std::shared_ptr<AllocatorBase> m_Allocator;
};

template<typename T, unsigned Alignment>
class StdAllocatorAligned {
public : 
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

public : 
    template<typename U>
    struct rebind {
        typedef StdAllocatorAligned<U, Alignment> other;
    };

public : 
    inline explicit StdAllocatorAligned(const std::shared_ptr<AllocatorBase>& allocator)
		: m_Allocator(allocator)
	{}
    
	inline ~StdAllocatorAligned() {}
    inline StdAllocatorAligned(const StdAllocatorAligned& rhs)
		: m_Allocator(rhs.m_Allocator)
	{}

    template<typename U>
    inline explicit StdAllocatorAligned(const StdAllocatorAligned<U, Alignment>& rhs)
		: m_Allocator(rhs.m_Allocator)
	{}

    inline pointer address(reference r) { return &r; }
    inline const_pointer address(const_reference r) { return &r; }

    inline pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer = 0) 
	{ 
		return reinterpret_cast<pointer>(m_Allocator->AllocateAligned(cnt * sizeof (T), Alignment)); 
    }
    inline void deallocate(pointer p, size_type) 
	{
		m_Allocator->DeallocateAligned(p);
    }

    inline size_type max_size() const 
	{ 
        return std::numeric_limits<size_type>::max() / sizeof(T);
	}

    inline void construct(pointer p, const T& t) { new(p) T(t); }
    inline void destroy(pointer p) { p->~T(); }

    inline bool operator==(StdAllocatorAligned const&) { return true; }
    inline bool operator!=(StdAllocatorAligned const& a) { return !operator==(a); }

private:
	template<typename U, unsigned A> friend class StdAllocatorAligned;
	std::shared_ptr<AllocatorBase> m_Allocator;
};
