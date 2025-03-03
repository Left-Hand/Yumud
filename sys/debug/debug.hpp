#pragma once

#include "sys/clock/clock.h"
#include "debugger.hpp"
#include <source_location>

namespace ymd{
// extern ymd::OutputStream & LOGGER;
extern ymd::__Debugger & DEBUGGER;
}

template<typename ... Args>
__fast_inline void DEBUG_PRINTLN(Args&&... args) {
    ymd::DEBUGGER.println(std::forward<Args>(args)...);
}


template<typename ... Args>
__fast_inline void DEBUG_PRINTLN_IDLE(Args&&... args) {
    if(ymd::DEBUGGER.pending() == 0) ymd::DEBUGGER.println(std::forward<Args>(args)...);
}

template<typename ... Args>
__fast_inline void DEBUG_PRINTS(Args&&... args) {
    ymd::DEBUGGER.prints(std::forward<Args>(args)...);
}


template<typename ... Args>
__fast_inline void DEBUG_PRINTT(Args&& ... args) {
    ymd::DEBUGGER.printt(std::forward<Args>(args)...);
}

template<typename ... Args>
__fast_inline void DEBUG_PRINT(Args&& ... args) {
    ymd::DEBUGGER.print(std::forward<Args>(args)...);
}

//目前还没办法做到不借助宏将变量名转为字符串
#define DEBUG_VALUE(value, ...) DEBUG_PRINTS("<", #value, ">\tis:", value, ##__VA_ARGS__);


// CATD模板 在构造时传递定位信息
// https://stackoverflow.com/questions/57547273/how-to-use-source-location-in-a-variadic-template-function
template <typename... Args>
struct DEBUG_SOURCE
{    
	DEBUG_SOURCE(Args &&... args, const std::source_location& loc = std::source_location::current()){
        {
            const auto guard = ymd::DEBUGGER.createGuard();
            ymd::DEBUGGER.reconf({
                .splitter = ", ",
                .radix = 10,
                .eps = 3,
                .flags = 0,
            });
            
            DEBUG_PRINT(loc.file_name(), 
            '(' ,loc.line() , ':' , loc.column() , ')'
            , loc.function_name() , ':');
        }
        
        DEBUG_PRINTLN(std::forward<Args>(args)...);
	}
};

template <typename... Args>
DEBUG_SOURCE(Args &&...) -> DEBUG_SOURCE<Args ...>;


template <typename... Args>
struct DEBUG_ERROR
{    
	DEBUG_ERROR(Args &&... args, const std::source_location& loc = std::source_location::current()){
        DEBUG_PRINT("[Err] ");
        DEBUG_SOURCE<Args...>(std::forward<Args>(args)..., loc);
	}
};

template <typename... Args>
DEBUG_ERROR(Args &&...) -> DEBUG_ERROR<Args...>;


template <typename... Args>
struct DEBUG_WARN
{    
	DEBUG_WARN(Args &&... args, const std::source_location& loc = std::source_location::current()){
        DEBUG_PRINT("[Warn] ");
        DEBUG_SOURCE<Args ...>(std::forward<Args>(args)..., loc);
	}
};

template <typename... Args>
DEBUG_WARN(Args &&...) -> DEBUG_WARN<Args ...>;


template <typename... Args>
struct PANIC
{    
	PANIC(Args &&... args, const std::source_location& loc = std::source_location::current()){
        DEBUG_ERROR<Args ...>(std::forward<Args>(args)..., loc);
        if constexpr(sizeof...(args)){
            delay(10);
        }

        DISABLE_INT;
        DISABLE_INT;
        exit(1);
	}
};

template <typename... Args>
PANIC(Args &&...) -> PANIC<Args ...>;


template <typename Texpr, typename... Args>
struct ASSERT{
private:
	bool result_;
public:
	constexpr ASSERT(Texpr && expr, Args &&... args, const std::source_location& loc = std::source_location::current()):
		result_(bool(expr)){
		if(!result_){
            DEBUG_ERROR<Args ...>(std::forward<Args>(args)..., loc);
			if constexpr(sizeof...(args)){
                delay(10);
			}
            DISABLE_INT;
            DISABLE_INT;
			exit(1);
		}
	}
	constexpr operator bool() const {return result_;}
};

template <typename Texpr, typename... Args>
ASSERT(Texpr&&, Args &&...) -> ASSERT<Texpr, Args...>;


#define TODO(...) PANIC("todo:", ##__VA_ARGS__)

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