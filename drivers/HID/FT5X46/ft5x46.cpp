#include "ft5x46.hpp"


#ifdef FT5X46_DEBUG_EN
#define FT5X46_TODO(...) TODO()
#define FT5X46_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define FT5X46_PANIC(...) PANIC{__VA_ARGS__}
#define FT5X46_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}


#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    PANIC{#x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define FT5X46_DEBUG(...)
#define FT5X46_TODO(...) PANIC_NSRC()
#define FT5X46_PANIC(...)  PANIC_NSRC()
#define FT5X46_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif