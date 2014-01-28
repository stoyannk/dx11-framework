// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "LogEnums.h"

namespace Logging
{
	class LogEntry
	{
	public:
		LogEntry() 
		{}
		
		LogEntry(Severity severity
			, Facility facility
			, const SYSTEMTIME& time
			, unsigned line
			, const std::string& file
			, const std::string& data)
			: m_Severity(severity)
			, m_Facility(facility)
			, m_Timestamp(time)
			, m_Line(line)
			, m_File(file)
			, m_Data(data)
		{}

	private:
		Severity m_Severity;
		Facility m_Facility;
		SYSTEMTIME m_Timestamp;
		unsigned m_Line;
		std::string m_File;
		std::string m_Data;

		friend std::ostream& operator<<(std::ostream& output, const LogEntry&);
	};
}