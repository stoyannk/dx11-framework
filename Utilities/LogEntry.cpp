// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "LogEntry.h"

namespace Logging
{
	std::ostream& operator<<(std::ostream& output, const Logging::LogEntry& entry)
	{
		std::ostringstream stream;

		const SYSTEMTIME& time = entry.m_Timestamp;

		stream << std::setfill('0') << std::setw(2);
		stream << time.wYear << "-" << time.wMonth << "-" << time.wDay << " ";
		stream << time.wHour << ":" << time.wMinute << ":" << time.wSecond << ":" << time.wMilliseconds << "; ";

		stream << LoggingNames::SeveritiesNames[entry.m_Severity] << "; ";

		stream << LoggingNames::FacilitiesNames[entry.m_Facility] << "; ";

		stream << entry.m_File << ":" << entry.m_Line << "; ";

		stream << entry.m_Data << std::endl;

		return output << stream.str();
	}
}