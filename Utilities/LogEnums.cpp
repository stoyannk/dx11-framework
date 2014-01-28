// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "Macros.h"

#include "LogEnums.h"

namespace Logging
{
	const char* LoggingNames::SeveritiesNames[] =
	{
		SMACRO_GENERATE_STRINGS(SEVERITIES)
	};

	const char* LoggingNames::FacilitiesNames[] =
	{
		SMACRO_GENERATE_STRINGS(FACILITIES)
	};
}