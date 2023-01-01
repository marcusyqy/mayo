#pragma once
#include <cassert>

#define STDX_ASSERT(condition, message) assert(!(condition) && message)
