#include "Si5351C.hpp"

#define Si5351C_DEBUG

#ifdef Si5351C_DEBUG
#undef Si5351C_DEBUG
#define Si5351C_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define Si5351C_DEBUG(...)
#endif

using namespace ymd::drivers;

