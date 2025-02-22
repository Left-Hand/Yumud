#pragma once

#include "sys/clock/clock.h"
#include "debugger.hpp"

namespace ymd{
extern ymd::OutputStream & LOGGER;
extern ymd::__Debugger & DEBUGGER;
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

#ifndef DEBUG_ERROR
#define DEBUG_ERROR(...) DEBUG_PRINTS("[E][", __FUNCTION__, ' ', __FILE__, ':', __LINE__ , ']', ##__VA_ARGS__)
#endif

#ifndef DEBUG_WARN
#define DEBUG_WARN(...) DEBUG_PRINTS("[W][", __FUNCTION__, ' ', __FILE__, ':', __LINE__ , ']', ##__VA_ARGS__)
#endif

#ifndef DEBUG_VALUE
#define DEBUG_VALUE(value, ...) DEBUG_PRINTS("<", #value, ">\tis:", value, ##__VA_ARGS__)
#endif


#define PANIC(...)\
do{\
    if constexpr((std::tuple_size_v<decltype(std::make_tuple(__VA_ARGS__))>)){\
        DEBUG_ERROR(__VA_ARGS__);\
        delay(10);\
    }\
    DISABLE_INT;\
    DISABLE_INT;\
    HALT;\
}while(false);\

#define ASSERT(cond, ...)\
({\
    bool __assert_result = (cond); \
    if (!__assert_result) {\
        PANIC(__VA_ARGS__);\
    }\
    __assert_result;\
})

#define TODO(...) do{PANIC("todo:", ##__VA_ARGS__)}while(false);

#define BREAKPOINT __nopn(1);

extern "C"{
__attribute__((used))
int _write(int file, char *buf, int len);

__attribute__((used))
void *_sbrk(ptrdiff_t incr);

__attribute__((used))
__attribute__((noreturn))
void _exit(int status);

__attribute__((used))
ssize_t _read(int fd, void *buf, size_t count);
}