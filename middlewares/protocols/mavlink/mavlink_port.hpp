#pragma once

namespace ymd::mavlink{

#define MAVLINK_DEBUG_ASSERT(cond, ...) if(bool(cond) == false) __builtin_trap();
}