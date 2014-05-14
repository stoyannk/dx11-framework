// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#include <windows.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

#include <unordered_map> 
#include <unordered_set> 

#include <memory>
#include <tuple>
#include <thread>
#include <mutex>

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_array.hpp>

#include <boost/circular_buffer.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <boost/cast.hpp>

#include "Assertions.h"
#include "Logger.h"
#include "SafeDestruction.h"