#pragma once

namespace ymd{
enum class VerifyLevel{
    NoCheck,
    Trap,
    Abort,
    Panic,
    Propagate
};

#define FLEX_HANDLE_NONE(...)\
if constexpr (verify_level == VerifyLevel::Propagate)\
    return None;\
else if constexpr (verify_level == VerifyLevel::Abort)\
    __builtin_abort();\
else if constexpr (verify_level == VerifyLevel::Panic)\
    PANIC{__VA_ARGS__};\
else{\
    __builtin_trap();}\


#define FLEX_HANDLE_ERR(error, ...)\
if constexpr (verify_level == VerifyLevel::Propagate)\
    return Err(error);\
else if constexpr (verify_level == VerifyLevel::Abort)\
    __builtin_abort();\
else if constexpr (verify_level == VerifyLevel::Panic)\
    PANIC{__VA_ARGS__};\
else{\
    __builtin_trap();}\


#define FLEX_INTERNAL_ASSERT_NONE(expr, ...) \
({\
    if constexpr (verify_level != VerifyLevel::NoCheck){\
        const bool expr_ = bool(expr);\
        if(expr_ == false) [[unlikely]]{\
            FLEX_HANDLE_NONE(__VA_ARGS__)\
        }\
    }\
})\

#define FLEX_EXTERNAL_ASSERT_NONE(expr, ...) \
({\
    const bool expr_ = bool(expr);\
    if(expr_ == false) [[unlikely]]{\
        FLEX_HANDLE_NONE(__VA_ARGS__)\
    }\
})\


#define FLEX_ASSERT_ERR(expr, error, ...) \
({\
    if constexpr (verify_level != VerifyLevel::NoCheck){\
        const bool expr_ = bool(expr);\
        if(expr_ == false) [[unlikely]]{\
            FLEX_HANDLE_ERR(error, __VA_ARGS__)\
        }\
    }\
})\

#define FLEX_RETURN_SOME(some) \
({\
    if constexpr (verify_level == VerifyLevel::Propagate)\
        return Some(some);\
    else return (some);\
})\

#define FLEX_RETURN_OK(okay) \
({\
    if constexpr (verify_level == VerifyLevel::Propagate)\
        return Ok(okay);\
    else return (okay);\
})\

#define FLEX_OPTION(obj_type) \
std::conditional_t<verify_level == VerifyLevel::Propagate , Option<obj_type>, obj_type>\

#define FLEX_RESULT(obj_type, err_type) \
std::conditional_t<verify_level == VerifyLevel::Propagate , Result<obj_type, err_type>, obj_type>\



#define FLEX_TRY_UNWRAP_OPTION(may, ...)\
({\
    const auto && _may = (may);\
    if(_may.is_none()){ [[unlikely]]\
        FLEX_HANDLE_NONE(__VA_ARGS__)}\
    _may.unwrap();\
})\

#define FLEX_TRY_UNWRAP_OPTION_TO_RESULT(may, error, ...)\
({\
    const auto && _may = (may);\
    if(_may.is_none()){ [[unlikely]]\
        {FLEX_HANDLE_ERR(error, __VA_ARGS__)}}\
    _may.unwrap();\
})\

#define FLEX_TRY_UNWRAP_RESULT(either, ...)\
({\
    const auto && _either = (either);\
    if(_either.is_err()) [[unlikely]]\
        {FLEX_HANDLE_ERR(__VA_ARGS__)}\
    _either.unwrap();\
})\

#define FLEX_TRY_UNWRAP_RESULT_TO_OPTION(either, ...)\
({\
    const auto && _either = (either);\
    if(_either.is_err()) [[unlikely]]\
        {FLEX_HANDLE_NONE(__VA_ARGS__)}\
    _either.unwrap();\
})\


}