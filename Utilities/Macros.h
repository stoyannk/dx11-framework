// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#define SMACRO_ITERATE_FOR_ENUM(r, data, elem) BOOST_PP_CAT(data, elem) ## ,

#define SMACRO_ITERATE_FOR_STRING(r, data, elem) BOOST_PP_STRINGIZE(elem) ## ,

#define SMACRO_GENERATE_ENUM(NAME, PREFIX, ENTRIES) \
	enum NAME \
	{ \
		BOOST_PP_SEQ_FOR_EACH(SMACRO_ITERATE_FOR_ENUM, PREFIX, ENTRIES) \
	}; 

#define SMACRO_GENERATE_STRINGS(ENTRIES) \
	BOOST_PP_SEQ_FOR_EACH(SMACRO_ITERATE_FOR_STRING, _, ENTRIES)
