#pragma once

namespace ymd{
enum class VerifyLevel{
    NoCheck,
    Trap,
    Abort,
    Panic,
    Propagate
};

#define CANOPEN_PANIC(...) ({})\

#define CANOPEN_FLEX_HANDLE_NONE(...)\
if constexpr (VERIFY_LEVEL == VerifyLevel::Propagate)\
    return None;\
else if constexpr (VERIFY_LEVEL == VerifyLevel::Abort)\
    __builtin_abort();\
else if constexpr (VERIFY_LEVEL == VerifyLevel::Panic)\
    CANOPEN_PANIC(__VA_ARGS__);\
else{\
    __builtin_trap();}\


#define CANOPEN_FLEX_HANDLE_ERR(error, ...)\
if constexpr (VERIFY_LEVEL == VerifyLevel::Propagate)\
    return Err(error);\
else if constexpr (VERIFY_LEVEL == VerifyLevel::Abort)\
    __builtin_abort();\
else if constexpr (VERIFY_LEVEL == VerifyLevel::Panic)\
    CANOPEN_PANIC(__VA_ARGS__);\
else{\
    __builtin_trap();}\


#define CANOPEN_FLEX_INTERNAL_ASSERT_NONE(expr, ...) \
({\
    if constexpr (VERIFY_LEVEL != VerifyLevel::NoCheck){\
        const bool expr_ = bool(expr);\
        if(expr_ == false) [[unlikely]]{\
            CANOPEN_FLEX_HANDLE_NONE(__VA_ARGS__)\
        }\
    }\
})\

#define CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(expr, ...) \
({\
    const bool expr_ = bool(expr);\
    if(expr_ == false) [[unlikely]]{\
        CANOPEN_FLEX_HANDLE_NONE(__VA_ARGS__)\
    }\
})\


#define CANOPEN_FLEX_ASSERT_ERR(expr, error, ...) \
({\
    if constexpr (VERIFY_LEVEL != VerifyLevel::NoCheck){\
        const bool expr_ = bool(expr);\
        if(expr_ == false) [[unlikely]]{\
            CANOPEN_FLEX_HANDLE_ERR(error, __VA_ARGS__)\
        }\
    }\
})\

#define CANOPEN_RETURN_SOME(some) \
({\
    if constexpr (VERIFY_LEVEL == VerifyLevel::Propagate)\
        return Some(some);\
    else return (some);\
})\

#define CANOPEN_FLEX_RETURN_OK(okay) \
({\
    if constexpr (VERIFY_LEVEL == VerifyLevel::Propagate)\
        return Ok(okay);\
    else return (okay);\
})\

#define CANOPEN_FLEX_OPTION(obj_type) \
std::conditional_t<VERIFY_LEVEL == VerifyLevel::Propagate , Option<obj_type>, obj_type>\

#define CANOPEN_FLEX_RESULT(obj_type, err_type) \
std::conditional_t<VERIFY_LEVEL == VerifyLevel::Propagate , Result<obj_type, err_type>, obj_type>\



#define CANOPEN_FLEX_TRY_UNWRAP_OPTION(may, ...)\
({\
    const auto && _may = (may);\
    if(_may.is_none()){ [[unlikely]]\
        CANOPEN_FLEX_HANDLE_NONE(__VA_ARGS__)}\
    _may.unwrap();\
})\

#define CANOPEN_FLEX_TRY_UNWRAP_OPTION_TO_RESULT(may, error, ...)\
({\
    const auto && _may = (may);\
    if(_may.is_none()){ [[unlikely]]\
        {CANOPEN_FLEX_HANDLE_ERR(error, __VA_ARGS__)}}\
    _may.unwrap();\
})\

#define CANOPEN_FLEX_TRY_UNWRAP_RESULT(either, ...)\
({\
    const auto && _either = (either);\
    if(_either.is_err()) [[unlikely]]\
        {CANOPEN_FLEX_HANDLE_ERR(__VA_ARGS__)}\
    _either.unwrap();\
})\

#define CANOPEN_FLEX_TRY_UNWRAP_RESULT_TO_OPTION(either, ...)\
({\
    const auto && _either = (either);\
    if(_either.is_err()) [[unlikely]]\
        {CANOPEN_FLEX_HANDLE_NONE(__VA_ARGS__)}\
    _either.unwrap();\
})\


}