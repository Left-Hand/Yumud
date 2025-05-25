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
class Borrow{
public:
    Borrow(T * ptr):
        ptr_(ptr){;}

    Borrow(std::nullptr_t) = delete;

    T & unwrap() const {return *ptr_;}
private:
    T * ptr_;
};

    
template<typename T>
class Option;
    
struct _None_t{
};

[[maybe_unused]] static inline _None_t None = {};
// [[maybe_unused]] static inline _None_t _ = {};

template<typename T>
struct Some{
public:
    constexpr Some(const T & val):val_(val){}
    constexpr Some(T && val):val_(std::move(val)){;}

    constexpr T & get(){
        return val_;
    }

    constexpr const T & get() const{
        return val_;
    }
private:
    using data_t = typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type;

    T val_;
};

template<>
struct Some<void>{
public:
};

template<typename T>
struct Some<T *>{
public:
    constexpr Some(std::nullptr_t) = delete;
    constexpr Some(T * val):val_(val){;}

    constexpr T * get(){
        return val_;
    }
private:
    T * val_;
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

    template<typename T2>
    requires (std::is_convertible_v<T2, TDecay>) and (not std::is_trivially_copy_assignable_v<TDecay>)
    constexpr Ok(T2 && val):val_(std::forward<std::decay_t<T2>>(val)){}

    template<typename T2>
    requires (std::is_convertible_v<T2, TDecay>) and (std::is_trivially_copy_assignable_v<TDecay>)
    constexpr Ok(T2 && val):val_(static_cast<std::decay_t<T2>>(val)){}

    // template<typename T2>
    // requires (std::is_convertible_v<T2, TDecay>)
    // constexpr Ok(T && val):val_(std::move<T>(val)){}
    // constexpr Ok(const T & val):val_((val)){}
    
    
    template<typename T2>
    requires (std::is_convertible_v<T2, TDecay>)
    constexpr Ok(const Ok<T2> & ok_val):val_(static_cast<TDecay>(ok_val.unwrap())){;}

    constexpr const TDecay & unwrap() const{
        return val_;
    }

private:
    TDecay val_;
};

template<>
struct Ok<void>{
public:
    constexpr Ok() = default;
};


namespace custom{
    
    // 非侵入式地添加隐式错误·类型转换
    template <typename From, typename To> 
    struct err_converter{};
}


template<typename E>
struct Err{
public:
    using EDecay = std::decay_t<E>;

    template<typename From>
    requires requires(From raw) {
        { custom::err_converter<Err<From>, Err<EDecay>>::convert(raw) } -> std::convertible_to<EDecay>;
    }
    [[nodiscard]] __fast_inline constexpr Err(const Err<From> & raw):
        val_(custom::err_converter<Err<From>, Err<EDecay>>::convert(raw)){}


    template<typename E2>
    requires (std::is_convertible_v<E2, EDecay>)
    constexpr Err(const Err<E2> & err):val_(static_cast<EDecay>(err.unwrap())){}
    

    
    template<typename E2>
    requires (std::is_convertible_v<E2, EDecay>)
    constexpr Err(E2 && err):val_(static_cast<EDecay>(err)){}

    constexpr const EDecay & unwrap() const{
        return val_;
    }
private:
    EDecay val_;
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


template<typename T>
class MATCH{
public:
    template<typename Fn, typename ... Args>
    constexpr void operator ()(const T kase, Fn && fn, Args && ...args){
        if(val_ == kase)return std::forward<Fn>(fn)();    
        else if constexpr(sizeof...(Args)) return this->operator()(std::forward<Args>(args)...);
    }

    template<typename Fn, typename ... Args>
    constexpr void operator ()(const _None_t, Fn && fn, Args && ...args){
        return std::forward<Fn>(fn)();    
    }

    MATCH(const T & val):val_(val){;}
private:    
    const T & val_;
};

template<typename T>
MATCH() -> MATCH<T>;


#define UNWRAP_OR_RETURN(x) if(const auto res = x; res.is_err()) return res;
#define UNWRAP_OR_RETURN_ERR(x) if(const auto res = x; res.is_err()) return Err(Error(res.unwrap_err()));

} // namespace ymd