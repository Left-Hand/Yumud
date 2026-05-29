#include "ft5x46.hpp"


#ifdef FT5X46_DEBUG_EN
#define FT5X46_TODO(...) TODO()
#define FT5X46_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define FT5X46_PANIC(...) PANIC{__VA_ARGS__}
#define FT5X46_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}




#define RAISE_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    PANIC{#x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define FT5X46_DEBUG(...)
#define FT5X46_TODO(...) PANIC_NSRC()
#define FT5X46_PANIC(...)  PANIC_NSRC()
#define FT5X46_ASSERT(cond, ...) ASSERT_NSRC(cond)


#define RAISE_ERR(x, ...) (x)
#endif