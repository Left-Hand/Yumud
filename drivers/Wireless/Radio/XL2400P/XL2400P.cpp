#include "xl2400p.hpp"

#define XL2400P_DEBUG_EN 0

#if XL2400P_DEBUG_EN == 1
#define XL2400P_TODO(...) TODO()
#define XL2400P_DEBUG(...) DEBUG_PRINTS(__VA_ARGS__);
#define XL2400P_PANIC(...) PANIC{__VA_ARGS__}
#define XL2400P_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define XL2400P_DEBUG(...)
#define XL2400P_TODO(...) PANIC_NSRC()
#define XL2400P_PANIC(...)  PANIC_NSRC()
#define XL2400P_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif