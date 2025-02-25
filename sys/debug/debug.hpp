#pragma once

#include "sys/clock/clock.h"
#include "debugger.hpp"
#include <source_location>

namespace ymd{
// extern ymd::OutputStream & LOGGER;
extern ymd::__Debugger & DEBUGGER;
}

#define DEBUG_PRINTLN(...) ymd::DEBUGGER.println(__VA_ARGS__);

#define DEBUG_PRINTLN_IDLE(...) if(ymd::DEBUGGER.pending() == 0) ymd::DEBUGGER.println(__VA_ARGS__);

#define DEBUG_PRINTS(...) ymd::DEBUGGER.prints(__VA_ARGS__);

#define DEBUG_PRINTT(...) ymd::DEBUGGER.printt(__VA_ARGS__);

#define DEBUG_PRINT(...) ymd::DEBUGGER.print(__VA_ARGS__);


#define DEBUG_SOURCE(...) __DEBUG_SOURCE(std::source_location::current(), ##__VA_ARGS__)

#define DEBUG_ERROR(...) DEBUG_PRINT("[Err] "); __DEBUG_SOURCE(std::source_location::current(), ##__VA_ARGS__);

#define DEBUG_WARN(...) DEBUG_PRINT("[Warn] "); __DEBUG_SOURCE(,std::source_location::current(), ##__VA_ARGS__);

#define DEBUG_VALUE(value, ...) DEBUG_PRINTS("<", #value, ">\tis:", value, ##__VA_ARGS__);


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
    bool __assert_result = static_cast<bool>(cond); \
    if (!__assert_result) {\
        PANIC(__VA_ARGS__);\
    }\
    __assert_result;\
})

template<typename ... Args>
void __DEBUG_SOURCE(const std::source_location location,Args && ... args){
    {
        const auto guard = ymd::DEBUGGER.createGuard();
        ymd::DEBUGGER.reconf({
            .splitter = ", ",
            .radix = 10,
            .eps = 3,
            .flags = 0,
        });
        
        DEBUG_PRINT(location.file_name(), 
        '(' ,location.line() , ':' , location.column() , ')'
        , location.function_name() , ':');
    }
    
    DEBUG_PRINTLN(std::forward<Args>(args)...);
}


#define TODO(...) do{PANIC("todo:", ##__VA_ARGS__)}while(false);

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