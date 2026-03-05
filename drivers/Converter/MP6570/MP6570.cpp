#include "mp6570_prelude.hpp"
#include "core/debug/debug.hpp"


#define MP6570_DEBUG

#ifdef MP6570_DEBUG
#undef MP6570_DEBUG
#define MP6570_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define MP6570_PANIC(...) PANIC(__VA_ARGS__)
#define MP6570_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define MP6570_DEBUG(...)
#define MP6570_PANIC(...)  PANIC_NSRC()
#define MP6570_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    ASSERT{false, #x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#define RETURN_ON_ERR(x) ({\
    if(const auto __res_return_on_err = (x); __res_return_on_err.is_err()){\
        return CHECK_RES(__res_return_on_err);\
    }\
});\


using namespace ymd;
using namespace ymd::drivers;