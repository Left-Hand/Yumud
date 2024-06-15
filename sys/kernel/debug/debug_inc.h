#ifndef __DEBUG_INC_H__

#define __DEBUG_INC_H__

#include "bus/bus_inc.h"
#include "sys/kernel/clock.h"

#define DEBUGGER uart1
#define LOGGER uart1

#ifndef VOFA_PRINT
#define VOFA_PRINT(...) LOGGER.println(__VA_ARGS__);
#endif


#ifndef DEBUG_PRINT
#define DEBUG_PRINT(...) DEBUGGER.println(__VA_ARGS__);
#endif

#ifndef DEBUG_TRAP
#define DEBUG_TRAP(...) DEBUG_PRINT("[T]", ##__VA_ARGS__);
#endif

#ifndef DEBUG_LOG
#define DEBUG_LOG(...) DEBUG_PRINT("[L]{", __FUNCTION__ ,',', '\t','\t',  __LINE__ , '}','\t', '\t', ##__VA_ARGS__)
#endif


#ifndef DEBUG_ERROR
#define DEBUG_ERROR(...) DEBUG_PRINT("[E]{", __FUNCTION__ ,',', '\t','\t',  __LINE__ , '}','\t', '\t', ##__VA_ARGS__)
#endif

#ifndef DEBUG_WARN
#define DEBUG_WARN(...) DEBUG_PRINT("[W]{", __FUNCTION__ ,',', '\t','\t',  __LINE__ , '}','\t', '\t', ##__VA_ARGS__)
#endif

#ifndef DEBUG_VALUE
#define DEBUG_VALUE(value, ...) DEBUG_PRINT("[V]", #value, "is", value, ##__VA_ARGS__)
#endif


#define RUN_TIME_DEBUG

// #define ASSERT(condition, ...)
// if(std::is_constant_evaluated() && std::is_con){ 
//     static_assert(condition, ##__VA_ARGS__);
// }else 
#if !defined(ASSERT) && defined(__cplusplus)
#ifdef RUN_TIME_DEBUG
#define ASSERT(condition, ...) \
if(!condition){\
    DEBUG_PRINT("[f]:", __LINE__, ##__VA_ARGS__);\
}
#endif
#else
#define ASSERT(condition, ...) if(std::is_constant_evaluated()) static_assert(condition, ##__VA_ARGS__);
#endif


#define ASSERT_WITH_CONTINUE(condition, ...) \
if(bool(condition) == false){\
    DEBUG_PRINT("[f]:", __LINE__, ##__VA_ARGS__);\
    continue;\
}

#define ASSERT_WITH_RETURN(condition, ...) \
if(bool(condition) == false){\
    DEBUG_PRINT("[f]:", __LINE__, ##__VA_ARGS__);\
    return;\
}

#define ASSERT_WITH_DOWN(condition, ...) \
if(bool(condition) == false){\
    DEBUG_PRINT("[f]:", __LINE__, ':', ##__VA_ARGS__);\
    while(true);\
}

#endif // __DEBUG_INC_H__