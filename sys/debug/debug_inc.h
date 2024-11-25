#pragma once

#include "sys/clock/clock.h"
#include "sys/stream/ostream.hpp"

namespace ymd{
extern ymd::OutputStream & LOGGER;
extern ymd::OutputStream & DEBUGGER;
}

#ifndef VOFA_PRINT
#define VOFA_PRINT(...) ymd::LOGGER.println(__VA_ARGS__);
#endif


#ifndef DEBUG_PRINTLN
#define DEBUG_PRINTLN(...) ymd::DEBUGGER.println(__VA_ARGS__);
#endif

#ifndef DEBUG_PRINTS
#define DEBUG_PRINTS(...) ymd::DEBUGGER.prints(__VA_ARGS__);
#endif


#ifndef DEBUG_TRAP
#define DEBUG_TRAP(...) DEBUG_PRINTS("[T]", ##__VA_ARGS__);
#endif

#ifndef DEBUG_LOG
#define DEBUG_LOG(...) DEBUG_PRINTS("[L][", __FUNCTION__, ' ', __FILE__, ':', __LINE__ , ']', ##__VA_ARGS__)
#endif


#ifndef DEBUG_ERROR
#define DEBUG_ERROR(...) DEBUG_PRINTS("[E][", __FUNCTION__, ' ', __FILE__, ':', __LINE__ , ']', ##__VA_ARGS__)
#endif

#ifndef DEBUG_WARN
#define DEBUG_WARN(...) DEBUG_PRINTS("[W][", __FUNCTION__, ' ', __FILE__, ':', __LINE__ , ']', ##__VA_ARGS__)
#endif

#ifndef DEBUG_VALUE
#define DEBUG_VALUE(value, ...) DEBUG_PRINTS("<", #value, ">\tis:", value, ##__VA_ARGS__)
#endif

// #define __COUNT_ARGS(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _n, X...) _n
// #define NARG(X...) __COUNT_ARGS(, ##X, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define NARG(...) (std::tuple_size_v<decltype(std::make_tuple(__VA_ARGS__))>)

#define PANIC(...)\
do{\
    if constexpr(NARG(__VA_ARGS__)){\
        DEBUG_ERROR(__VA_ARGS__);\
        delay(10);\
    }\
    __disable_irq();\
    __disable_irq();\
    HALT;\
}while(false);\



#define ASSERT(condition, ...) \
({ \
    bool _cond = (bool)(condition); \
    if (!_cond) { \
        PANIC(__VA_ARGS__); \
    } \
    _cond; \
})

#define ASSERT_WITH_CONTINUE(condition, ...) \
if(bool(condition) == false){\
    DEBUG_PRINTLN("[f]:", __LINE__, ##__VA_ARGS__);\
    continue;\
}

#define ASSERT_WITH_RETURN(condition, ...) \
if(bool(condition) == false){\
    DEBUG_PRINTLN("[f]:", __LINE__, ##__VA_ARGS__);\
    return;\
}

#define ASSERT_WITH_HALT(condition, ...) \
if(bool(condition) == false){\
    DEBUG_PRINTLN("[f]:", __LINE__, ':', ##__VA_ARGS__);\
    PANIC(__VA_ARGS__);\
}\

#define BREAKPOINT __nopn(1);

extern "C"{
__attribute__((used)) int _write(int fd, char *buf, int size);
__attribute__((used)) void *_sbrk(ptrdiff_t incr);

__attribute__((used)) void _exit(int status);
__attribute__((used)) ssize_t _read(int fd, void *buf, size_t count);
}