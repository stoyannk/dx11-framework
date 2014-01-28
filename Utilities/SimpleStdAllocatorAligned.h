// Copyright (c) 2012-2013, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

template<typename T, unsigned Alignment>
class SimpleStdAllocatorAligned {
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
		typedef SimpleStdAllocatorAligned<U, Alignment> other;
    };

public : 
	inline SimpleStdAllocatorAligned(){}
	inline ~SimpleStdAllocatorAligned() {}
	inline SimpleStdAllocatorAligned(const SimpleStdAllocatorAligned& rhs) {}

    template<typename U>
	inline explicit SimpleStdAllocatorAligned(const SimpleStdAllocatorAligned<U, Alignment>& rhs){}

    inline pointer address(reference r) { return &r; }
    inline const_pointer address(const_reference r) { return &r; }

    inline pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer = 0) 
	{ 
		return reinterpret_cast<pointer>(_aligned_malloc(cnt * sizeof (T), Alignment));
    }
    inline void deallocate(pointer p, size_type) 
	{
		_aligned_free(p);
    }

    inline size_type max_size() const 
	{ 
        return std::numeric_limits<size_type>::max() / sizeof(T);
	}

    inline void construct(pointer p, const T& t) { new(p) T(t); }
    inline void destroy(pointer p) { p->~T(); }

	inline bool operator==(SimpleStdAllocatorAligned const&) { return true; }
	inline bool operator!=(SimpleStdAllocatorAligned const& a) { return !operator==(a); }

private:
	template<typename U, unsigned A> friend class SimpleStdAllocatorAligned;
};
