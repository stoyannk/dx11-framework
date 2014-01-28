// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

template<typename T>
void SafeDelete(T*& ptr)
{
	delete ptr;
	ptr = nullptr;
}

template<typename T>
void SafeRelease(T*& ptr)
{
	if(ptr)
	{
		ptr->Release();
		ptr = nullptr;
	}
}

template <typename T>
class ReleaseGuard
{
public:
	ReleaseGuard() : m_Pointer(nullptr)
	{}

	explicit ReleaseGuard(T* ptr) : m_Pointer(ptr)
	{}

	ReleaseGuard(ReleaseGuard&& rhs)
	{
		*this = std::move(rhs);
	}

	ReleaseGuard& operator=(ReleaseGuard&& rhs)
	{
		if(this != &rhs)
		{
			m_Pointer = rhs.m_Pointer;
			rhs.m_Pointer = nullptr;
		}

		return *this;
	}

	void Set(T* ptr)
	{
		SafeRelease(m_Pointer);
		m_Pointer = ptr;
	}

	T* operator->() const
	{
		return m_Pointer;
	}

	T* Get() const
	{
		return m_Pointer;
	}

	T* const* GetConstPP() const
	{
		return &m_Pointer;
	}

	T** Receive()
	{
		SafeRelease(m_Pointer);
		return &m_Pointer;
	}

	T* Denounce()
	{
		T* ptr = m_Pointer;
		m_Pointer = nullptr;
		return ptr;
	}

	~ReleaseGuard()
	{
		SafeRelease(m_Pointer);
	}

private:
	T* m_Pointer;
	
	ReleaseGuard(const ReleaseGuard&);
	ReleaseGuard& operator=(const ReleaseGuard&);
};

template <typename T>
class ArrayDeleter
{
public:
    void operator() (T* d) const
    {
        delete[] d;
    }
};