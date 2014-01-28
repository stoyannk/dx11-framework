// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "Macros.h"

namespace Logging
{
	#define SEVERITIES \
		(Trace) \
		(Debug) \
		(Info) \
		(Warning) \
		(Error) \
		(Count)

	#define FACILITIES \
		(None) \
		(Rendering) \
		(Subdivision) \
		(OpenCL) \
		(Voxels) \
		(Count)
	
	SMACRO_GENERATE_ENUM(Severity, Sev_, SEVERITIES)

	SMACRO_GENERATE_ENUM(Facility, Fac_, FACILITIES)

	struct LoggingNames
	{
		static const char* FacilitiesNames[Fac_Count + 1];
		static const char* SeveritiesNames[Sev_Count + 1];
	};
}