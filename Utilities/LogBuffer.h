// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "LogEntry.h"

namespace Logging
{
	class LogBuffer
	{
	public:
		LogBuffer(size_t size);
		~LogBuffer();

		// Thread-safe - many threads are allowed to input
		bool AddEntry(const LogEntry& entry);
		
		// Not thread safe - access by a single thread only!
		bool RemoveEntry(LogEntry& entry);

		HANDLE GetLogEvent() const;

	private:
		size_t Next(size_t i) const
		{
			return ++i %= m_Size;
		}

		size_t m_Size;
		boost::scoped_array<LogEntry> m_Entries;

		size_t m_In;
		size_t m_Out;

		CRITICAL_SECTION m_InputLock;
		HANDLE m_LogEvent;

		static const unsigned int SPIN_COUNT = 5000;
	};

}