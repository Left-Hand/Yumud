#pragma once

#include <optional>
#include <type_traits>

namespace ymd {

// Helper template to unwrap optional types
template <typename T>
struct __unwrap_helper {
};

// Specialization for std::optional
template <typename T>
struct __unwrap_helper<std::optional<T>> {
    // Unwrap a non-const rvalue optional
    static constexpr T&& unwrap(std::optional<T>&& opt) {
        return std::move(opt.value());
    }

    // Unwrap a const lvalue optional
    static constexpr const T& unwrap(const std::optional<T>& opt) {
        return opt.value();
    }

    // Return std::nullopt for unexpected cases
    static constexpr std::nullopt_t unexpected(const std::optional<T>& opt) {
        return std::nullopt;
    }

    // Return std::nullopt for unexpected cases
    static constexpr std::nullopt_t unexpected(std::optional<T> && opt) {
        return std::nullopt;
    }
};

// Macro to simplify unwrapping
#define UNWRAP(expr) \
    ({ \
        const auto result = (expr); \
        using helper = ymd::__unwrap_helper<std::decay_t<decltype(result)>>; \
        if (unlikely(!helper::is_ok(result))) { \
            return helper::unexpected(result); \
        } \
        helper::unwrap(std::move(result)); \
    })



    
template<typename T>
class Option;
    
struct _None_t{
};

static inline _None_t None = {};

template<typename T>
struct Some{
public:
    constexpr Some(const T & val):val_(val){}
    constexpr Some(T && val):val_(std::move(val)){;}

    constexpr T & operator*(){
        return val_;
    }
private:
    T val_;
};

template<>
struct Some<void>{
public:
};

//CTAD
template<typename T>
Some(T && val) -> Some<std::decay_t<T>>;

template<typename TDummy = void>
Some() -> Some<void>;

template<typename T, typename E>
class Result;

template<typename T = void>
struct Ok{
public:
    using TDecay = std::decay_t<T>;

    constexpr Ok(auto val):val_((val)){}

    constexpr operator T() const{
        return val_;
    }

    constexpr const T & operator *(){
        return val_;
    }

    template <typename E>
    operator Result<T, E>() const {
        return Result<T, E>(Ok<T>(val_));
    }

private:
    TDecay val_;
};

template<>
struct Ok<void>{
public:
    constexpr Ok() = default;
};

template<typename E>
struct Err{
public:
    template<typename E2>
    constexpr Err(const Err<E2> & err):val_(std::forward<E>(err.val_)){}
    
    template<typename E2>
    constexpr Err(Err<E2> && err):val_(std::forward<E>(err.val_)){}

    template<typename E2>
    constexpr Err(const E2 & err):val_(err){}

    template<typename E2>
    constexpr Err(E2 && err):val_(std::forward<E2>(err)){}

    constexpr operator E() const{
        return val_;
    }

    constexpr const E & operator *(){
        return val_;
    }

    
    // template <typename T>
    // operator Result<T, E>() const {
    //     return Result<T, E>(Err<E>(val_));
    // }
private:
    E val_;
};


template<>
struct Err<void>{
public:
    constexpr Err() = default;
};


template<typename T>
Ok(T && val) -> Ok<std::decay_t<T>>;

template<typename TDummy = void>
Ok() -> Ok<void>;

template<typename E>
Err(E && val) -> Err<std::decay_t<E>>;

template<typename TDummy = void>
Err() -> Err<void>;


template<std::size_t Size>
struct size_to_int;

template<>
struct size_to_int<1> {
    using type = int8_t;
};

template<>
struct size_to_int<2> {
    using type = int16_t;
};

template<>
struct size_to_int<4> {
    using type = int32_t;
};

template<>
struct size_to_int<8> {
    using type = int64_t;
};

template<std::size_t Size>
struct size_to_uint;

template<>
struct size_to_uint<1> {
    using type = int8_t;
};

template<>
struct size_to_uint<2> {
    using type = int16_t;
};

template<>
struct size_to_uint<4> {
    using type = int32_t;
};

template<>
struct size_to_uint<8> {
    using type = int64_t;
};

template<size_t Size>
using size_to_int_t = typename size_to_int<Size>::type;

template<size_t Size>
using size_to_uint_t = typename size_to_uint<Size>::type;

} // namespace ymd