#include "xn297l.hpp"


#define XN297L_DEBUG_EN 0

#if XN297L_DEBUG_EN == 1
#define XN297L_TODO(...) TODO()
#define XN297L_DEBUG(...) DEBUG_PRINTS(__VA_ARGS__);
#define XN297L_PANIC(...) PANIC{__VA_ARGS__}
#define XN297L_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define XN297L_DEBUG(...)
#define XN297L_TODO(...) PANIC_NSRC()
#define XN297L_PANIC(...)  PANIC_NSRC()
#define XN297L_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif