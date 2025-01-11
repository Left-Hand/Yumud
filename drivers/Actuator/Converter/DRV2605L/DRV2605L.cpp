#include "DRV2605L.hpp"


#define DRV2605_DEBUG

#ifdef DRV2605_DEBUG
#undef DRV2605_DEBUG
#define DRV2605_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define DRV2605_PANIC(...) PANIC(__VA_ARGS__)
#define DRV2605_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define DRV2605_DEBUG(...)
#define DRV2605_PANIC(...)  PANIC()
#define DRV2605_ASSERT(cond, ...) ASSERT(cond)
#endif


using namespace ymd::drivers;

