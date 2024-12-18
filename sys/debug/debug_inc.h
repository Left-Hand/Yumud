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

#define NARG(...) (std::tuple_size_v<decltype(std::make_tuple(__VA_ARGS__))>)

#define PANIC(...)\
do{\
    if constexpr(NARG(__VA_ARGS__)){\
        DEBUG_ERROR(__VA_ARGS__);\
        delay(10);\
    }\
    DISABLE_INT;\
    DISABLE_INT;\
    HALT;\
}while(false);\

template<typename... Args>
bool __assert_impl(bool cond, Args&&... args) {
    if (!cond) {
        PANIC(std::forward<Args>(args)...);
    }
    return cond;
}

#define ASSERT(condition, ...) __assert_impl((bool)(condition), ##__VA_ARGS__);

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
#define TODO(...) do{PANIC("todo", ##__VA_ARGS__)}while(false);

extern "C"{
__attribute__((used)) int _write(int fd, char *buf, int size);
__attribute__((used)) void *_sbrk(ptrdiff_t incr);

__attribute__((used)) void _exit(int status);
__attribute__((used)) ssize_t _read(int fd, void *buf, size_t count);
}