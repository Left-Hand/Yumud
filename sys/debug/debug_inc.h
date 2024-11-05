#pragma once

#include "hal/bus/uart/uarthw.hpp"
#include "src/defines/user_defs.h"

#ifndef VOFA_PRINT
#define VOFA_PRINT(...) LOGGER.println(__VA_ARGS__);
#endif


#ifndef DEBUG_PRINTLN
#define DEBUG_PRINTLN(...) DEBUGGER.println(__VA_ARGS__);
#endif

#ifndef DEBUG_PRINTS
#define DEBUG_PRINTS(...) DEBUGGER.prints(__VA_ARGS__);
#endif


#ifndef DEBUG_TRAP
#define DEBUG_TRAP(...) DEBUG_PRINTS("[T]", ##__VA_ARGS__);
#endif

#ifndef DEBUG_LOG
#define DEBUG_LOG(...) DEBUG_PRINTS("[L]{", __FUNCTION__ ,',', __LINE__ , '}', ##__VA_ARGS__)
#endif


#ifndef DEBUG_ERROR
#define DEBUG_ERROR(...) DEBUG_PRINTS("[E]{", __FUNCTION__ ,',',  __LINE__ , '}', ##__VA_ARGS__)
#endif

#ifndef DEBUG_WARN
#define DEBUG_WARN(...) DEBUG_PRINTS("[W]{", __FUNCTION__ ,',',  __LINE__ , '}', ##__VA_ARGS__)
#endif

#ifndef DEBUG_VALUE
#define DEBUG_VALUE(value, ...) DEBUG_PRINTS("<", #value, ">\tis:", value, ##__VA_ARGS__)
#endif


#define PANIC(...)\
do{\
    if constexpr(sizeof(std::make_tuple(__VA_ARGS__))){\
        DEBUG_ERROR(__VA_ARGS__);\
    }\
    HALT;\
}while(false);\



#define RUN_TIME_DEBUG

#if !defined(ASSERT) && defined(__cplusplus)
#ifdef RUN_TIME_DEBUG
#define ASSERT(condition, ...) \
if(!condition){\
    DEBUG_PRINTLN("[f]:", __LINE__, ##__VA_ARGS__);\
}
#endif
#else
#define ASSERT(condition, ...) if(std::is_constant_evaluated()) static_assert(condition, ##__VA_ARGS__);
#endif


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
    __disable_irq();\
    __disable_irq();\
    delay(1);\
    HALT;\
}

#define BREAKPOINT __nopn(1);

extern "C"{

__attribute__((used)) int _write(int fd, char *buf, int size);
__attribute__((used)) void *_sbrk(ptrdiff_t incr);

__attribute__((used)) void _exit(int status);
__attribute__((used)) ssize_t _read(int fd, void *buf, size_t count);
}