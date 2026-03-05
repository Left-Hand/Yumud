#include "bq769x0_prelude.hpp"
#include "core/debug/debug.hpp"

#define BQ769X0_DEBUG_EN 0

#if BQ769X0_DEBUG_EN

#define BQ769X0_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define BQ769X0_PANIC(...) PANIC{__VA_ARGS__}
#define BQ769X0_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define BQ769X0_DEBUG(...)
#define BQ769X0_PANIC(...)  PANIC_NSRC()
#define BQ769X0_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

using namespace ymd;
using namespace ymd::drivers;