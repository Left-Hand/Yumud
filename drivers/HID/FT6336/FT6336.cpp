#include "FT6336.hpp"

#define FT6636_DEBUG

#ifdef FT6636_DEBUG
#undef FT6636_DEBUG
#define FT6636_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define FT6636_PANIC(...) PANIC(__VA_ARGS__)
#define FT6636_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define FT6636_DEBUG(...)
#define FT6636_PANIC(...)  PANIC()
#define FT6636_ASSERT(cond, ...) ASSERT(cond)
#endif

using namespace ymd::drivers;



