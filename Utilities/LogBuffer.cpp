// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "LogBuffer.h"

namespace Logging
{

	LogBuffer::LogBuffer(size_t size)
		: m_Size(size)
		, m_Entries(new LogEntry[size])
	{
		m_In = 0;
		m_Out = m_In;

		::InitializeCriticalSectionAndSpinCount(&m_InputLock, SPIN_COUNT);

		m_LogEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}

	LogBuffer::~LogBuffer()
	{
		::DeleteCriticalSection(&m_InputLock);
		::CloseHandle(m_LogEvent);
	}

	// Thread-safe - many threads are allowed to input
	bool LogBuffer::AddEntry(const LogEntry& entry)
	{
		::EnterCriticalSection(&m_InputLock);
		
		m_Entries[m_In] = entry;

		const size_t next = Next(m_In);
		if(next != m_Out)
		{
			m_In = next;			
		}
		else
		{
			assert(false && "Log buffer full - possible loss of data and undefined behavior");
		}

		::LeaveCriticalSection(&m_InputLock);

		::SetEvent(m_LogEvent);

		return true;
	}

	// Not thread safe - access by a single thread only!
	bool LogBuffer::RemoveEntry(LogEntry& entry)
	{
		size_t currentIn = m_In;

		if(m_Out == currentIn)
		{
			return false;
		}

		entry = m_Entries[m_Out];
		m_Out = Next(m_Out);

		return true;
	}

	HANDLE LogBuffer::GetLogEvent() const
	{
		return m_LogEvent;
	}
}