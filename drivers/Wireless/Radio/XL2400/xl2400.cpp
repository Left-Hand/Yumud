#include "xl2400.hpp"


#define XL2400_DEBUG_EN 0

#if XL2400_DEBUG_EN == 1
#define XL2400_TODO(...) TODO()
#define XL2400_DEBUG(...) DEBUG_PRINTS(__VA_ARGS__);
#define XL2400_PANIC(...) PANIC{__VA_ARGS__}
#define XL2400_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define XL2400_DEBUG(...)
#define XL2400_TODO(...) PANIC_NSRC()
#define XL2400_PANIC(...)  PANIC_NSRC()
#define XL2400_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

using namespace ymd::drivers;
using namespace ymd;

