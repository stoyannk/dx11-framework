// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "Logger.h"

namespace Logging
{
	Logger* Logger::Instance = nullptr;

	void Logger::Initialize(size_t size)
	{
		Instance = new Logger(size);
	}

	void Logger::Deinitialize()
	{
		delete Instance;
	}
	
	// Gains ownership
	void Logger::AddTarget(std::ostream* stream, bool takeOwnership)
	{
		std::lock_guard<std::mutex> lock(m_TargetsMutex); 
		m_Targets.push_back(std::make_pair(stream, takeOwnership));
	}

	Logger& Logger::Get()
	{
		return *Instance;
	}

	Logger::Logger(size_t size)
		: m_Buffer(new LogBuffer(size))
		, m_Run(true)
		, m_MinimalLogSeverity(0)
	{
		m_LoggingThread = std::thread(RealLogger(this));

		m_BufferLoggedEvent = m_Buffer->GetLogEvent();

		m_EndEvent = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
	}

	Logger::~Logger()
	{
		m_Run = false;
		::SetEvent(m_EndEvent);

		m_LoggingThread.join();

		::CloseHandle(m_EndEvent);

		std::lock_guard<std::mutex> lock(m_TargetsMutex);
		for(auto it = m_Targets.cbegin(); it != m_Targets.cend(); ++it)
		{
			if(it->second)
			{
				delete it->first;
			}
		}

		m_Targets.clear();
	}

	void Logger::Log(Severity severity
					, Facility facility
					, unsigned line
					, const std::string& file
					, const std::string& data)
	{
		if (m_MinimalLogSeverity > int(severity))
			return;

		SYSTEMTIME time;
		::GetSystemTime(&time);

		m_Buffer->AddEntry(LogEntry(severity
									, facility
									, time
									, line
									, file
									, data));
	}

	void Logger::RealLogger::operator()()
	{
		HANDLE handles[] = {m_Parent->m_BufferLoggedEvent, m_Parent->m_EndEvent};
		
		while(m_Parent->m_Run)
		{
			::WaitForMultipleObjects(ARRAYSIZE(handles), handles, FALSE, INFINITE);

			Log();
		}
	}

	void Logger::RealLogger::Log() const
	{
		LogEntry entry;
		std::lock_guard<std::mutex> lock(m_Parent->m_TargetsMutex);
		while(m_Parent->m_Buffer->RemoveEntry(entry))
		{
			for(auto it = m_Parent->m_Targets.begin(); it != m_Parent->m_Targets.end(); ++it)
			{
				*(it->first) << entry;
			}
		}
	}
}