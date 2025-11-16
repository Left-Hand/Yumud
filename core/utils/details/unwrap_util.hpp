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
    constexpr Some(T * ptr):ptr_(ptr){;}

    constexpr T * get() const {
        return ptr_;
    }

    constexpr T & deref() const {
        return *ptr_;
    }
private:
    T * ptr_;
};

//CTAD
template<typename T>
Some(T && ptr) -> Some<std::decay_t<T>>;

template<typename TDummy = void>
Some() -> Some<void>;

template<typename T, typename E>
class Result;

template<typename T = void>
struct Ok {
public:
    using value_type = std::decay_t<T>;
    
    // 主构造函数 - 使用完美转发
    template<typename U>
    requires (std::is_constructible_v<value_type, U&&> && 
             !std::is_same_v<std::remove_cvref_t<U>, Ok>)
    constexpr explicit(!std::is_convertible_v<U&&, value_type>) 
    Ok(U&& value) 
        noexcept(std::is_nothrow_constructible_v<value_type, U&&>)
        : value_(std::forward<U>(value)) {}
    
    // 拷贝构造函数
    template<typename U>
    requires (std::is_constructible_v<value_type, const U&> && 
             std::is_convertible_v<const U&, value_type>)
    constexpr Ok(const Ok<U>& other)
        noexcept(std::is_nothrow_constructible_v<value_type, const U&>)
        : value_(other.value_) {}
    
    // 移动构造函数  
    template<typename U>
    requires (std::is_constructible_v<value_type, U&&> && 
             std::is_convertible_v<U&&, value_type>)
    constexpr Ok(Ok<U>&& other)
        noexcept(std::is_nothrow_constructible_v<value_type, U&&>)
        : value_(std::move(other).get()) {}
    
    // 默认构造函数 (仅当 T 可默认构造时)
    constexpr Ok() requires std::is_default_constructible_v<value_type>
        : value_() {}
    
    // 访问器
    [[nodiscard]] constexpr const value_type& get() const & noexcept {
        return value_;
    }
    
    [[nodiscard]] constexpr value_type& get() & noexcept {
        return value_;
    }
    
    [[nodiscard]] constexpr value_type&& get() && noexcept {
        return std::move(value_);
    }
    
    [[nodiscard]] constexpr const value_type&& get() const && noexcept {
        return std::move(value_);
    }

private:
    value_type value_;
};

// void 特化版本
template<>
struct Ok<void> {
    constexpr Ok() = default;
    
    template<typename U>
    constexpr Ok(U&&) = delete; // 防止误用
    
    constexpr void get() const noexcept {}
};


namespace custom{
    
    // 非侵入式地添加隐式错误·类型转换
    template <typename From, typename To> 
    struct err_converter{};
}


template<typename E = void>
struct Err {
public:
    using error_type = std::decay_t<E>;
    
    // 主构造函数 - 完美转发
    template<typename U>
    requires (std::is_constructible_v<error_type, U&&> && 
             !std::is_same_v<std::remove_cvref_t<U>, Err>)
    constexpr explicit(!std::is_convertible_v<U&&, error_type>)
    Err(U&& error) 
        noexcept(std::is_nothrow_constructible_v<error_type, U&&>)
        : value_(std::forward<U>(error)) {}
    
    // 自定义转换器构造函数
    template<typename From>
    requires requires(const Err<From>& raw) {
        { custom::err_converter<Err<From>, Err<error_type>>::convert(raw) } 
            -> std::convertible_to<error_type>;
        requires custom::err_converter<Err<From>, Err<error_type>>::is_specialized;
    }
    [[nodiscard]] constexpr Err(const Err<From>& other)
        noexcept(noexcept(custom::err_converter<Err<From>, Err<error_type>>::convert(other)))
        : value_(custom::err_converter<Err<From>, Err<error_type>>::convert(other)) {}
    
    // 拷贝构造函数 - 同类型
    template<typename U = E>
    requires (std::is_constructible_v<error_type, const U&>)
    constexpr Err(const Err& other)
        noexcept(std::is_nothrow_constructible_v<error_type, const U&>)
        : value_(other.value_) {}
    
    // 移动构造函数 - 同类型  
    template<typename U = E>
    requires (std::is_constructible_v<error_type, U&&>)
    constexpr Err(Err&& other)
        noexcept(std::is_nothrow_constructible_v<error_type, U&&>)
        : value_(std::move(other.value_)) {}
    
    // 跨类型拷贝构造函数
    template<typename U>
    requires (std::is_constructible_v<error_type, const U&> && 
             std::is_convertible_v<const U&, error_type> &&
             !std::is_same_v<U, E>)
    constexpr Err(const Err<U>& other)
        noexcept(std::is_nothrow_constructible_v<error_type, const U&>)
        : value_(other.get()) {}
    
    // 跨类型移动构造函数
    template<typename U>
    requires (std::is_constructible_v<error_type, U&&> && 
             std::is_convertible_v<U&&, error_type> &&
             !std::is_same_v<U, E>)
    constexpr Err(Err<U>&& other)
        noexcept(std::is_nothrow_constructible_v<error_type, U&&>)
        : value_(std::move(other).get()) {}
    
    // 默认构造函数 (仅当 E 可默认构造时)
    constexpr Err() requires std::is_default_constructible_v<error_type>
        : value_() {}
    
    // 访问器 - 完整的值类别支持
    [[nodiscard]] constexpr const error_type& get() const & noexcept {
        return value_;
    }
    
    [[nodiscard]] constexpr error_type& get() & noexcept {
        return value_;
    }
    
    [[nodiscard]] constexpr error_type&& get() && noexcept {
        return std::move(value_);
    }
    
    [[nodiscard]] constexpr const error_type&& get() const && noexcept {
        return std::move(value_);
    }

private:
    error_type value_;
};

// void 特化版本
template<>
struct Err<void> {
    constexpr Err() = default;
    
    template<typename U>
    constexpr Err(U&&) = delete; // 防止误用
    
    constexpr void get() const noexcept {}
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