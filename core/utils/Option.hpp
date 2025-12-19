#pragma once

#include "core/platform.hpp"
#include "core/debug/debug.hpp"
#include "core/tmp/functor.hpp"

#include "details/unwrap_util.hpp"
#include <variant>
#include <source_location>

namespace ymd{


namespace custom{
    template <typename T, typename S> 
    struct option_converter{};

    // 非侵入式地添加隐式类型转换
    // T为正确类型 S为源类型

    // eg:
    // template<>
    // struct option_converter<hal::HalResult, hal::HalResult> {
    //     static Option<hal::HalResult> convert(const hal::HalResult & res){
    //         if(res.ok()) return Some(res);
    //         else return None; 
    //     }
    // };
}


template<typename T>
static constexpr bool is_option_v = false;

template<typename T>
static constexpr bool is_option_v<Option<T>> = true;

namespace details{
template<typename T>
struct _option_type{
};

template<typename T>
struct _option_type<Option<T>>{
    using type = T;
};

}

template<typename T>
using option_type_t = details::_option_type<T>::type;

template<typename T>
class [[nodiscard]] Option{
public:
    using value_type = T;
private:
    using data_t = typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type;
    // T value_;


    // data_t storage_;

    struct Empty {};
    union {
        Empty empty_;
        T storage_;
    };

    bool exists_;

    constexpr T & get(){return storage_;}
    constexpr const T & get() const {return storage_;}
public:
    [[nodiscard]] constexpr 
    Option(_None_t):
        exists_(false)
    {}

    [[nodiscard]] constexpr 
    Option(const Some<T> & something):
        exists_(true)
        {
            std::construct_at(&storage_, something.get());
        }

    template<typename U>
    requires std::convertible_to<U, T>
    [[nodiscard]] constexpr 
    Option(const Some<U> & something):
        exists_(true)
        {
            std::construct_at(&storage_, static_cast<T>(something.get()));
        }

    [[nodiscard]] constexpr 
    Option(const Option<T> & other) = default;

    constexpr 
    Option & operator = (const Option<T> & other) = default;

    
    [[nodiscard]] constexpr 
    Option(Option<T> && other):
        exists_(other.exists_)
    {
        this->exists_ = other.exists_;
        
        if(other.exists_){
            std::construct_at(&storage_, other.storage_);
            std::destroy_at(&other.storage_);
        }
        other.exists_ = false;

    }

        
    constexpr 
    Option & operator = (Option<T> && other){
        if(exists_) std::destroy_at(&storage_);
        exists_ = other.exists_;
        if(other.exists_) std::construct_at(&storage_, std::move(other.storage_));
        other.exists_ = false;
        return *this;
    }

    constexpr 
    Option & operator = (const Some<T> & something){ 
        if (exists_) std::destroy_at(&storage_);
        std::construct_at(&storage_, something.get());
        exists_ = true;
        return *this;
    }

    constexpr ~Option() {
        if (exists_) {
            storage_.~T();
        }
    }


    template<typename S>
    requires requires(S s) {
        { custom::option_converter<T, S>::convert(s) } -> std::convertible_to<Option<T>>;
    }
    [[nodiscard]] __fast_inline constexpr 
    Option(const S & other):Option(custom::option_converter<T, S>::convert(other)){}
    [[nodiscard]] __fast_inline constexpr bool 
    is_some() const{ return exists_; }
    [[nodiscard]] __fast_inline constexpr bool 
    is_none() const{ return !exists_; }

    [[nodiscard]] __fast_inline constexpr bool 
    is_value(const T value) const{
        if(not exists_) return false;
        return value == unwrap();
    }

    [[nodiscard]] __fast_inline constexpr const T & 
    value_or(const T & default_value) const{
        return exists_ ? get() : default_value;
    }

    [[nodiscard]] __fast_inline constexpr const T & 
    unwrap_loc(const std::source_location & loc = std::source_location::current()) const {
        if(unlikely(exists_ == false)){
            __PANIC_EXPLICIT_SOURCE(loc);
        }
        return get();
    }

    [[nodiscard]] __fast_inline constexpr const T & 
    unwrap() const {
        if(unlikely(exists_ == false)) __builtin_trap();
        return get();
    }

    [[nodiscard]] __fast_inline constexpr T & 
    unwrap(){
        if(unlikely(exists_ == false)) __builtin_trap();
        return get();
    }

    [[nodiscard]] __fast_inline constexpr const T & 
    examine(const std::source_location loca = std::source_location::current()) const {
        if (unlikely(!is_some())) {
            __PANIC_EXPLICIT_SOURCE(loca);
        }
        return get();
    }


    template<typename ... Args>
    [[nodiscard]] __fast_inline constexpr const T & 
    expect(Args && ... args) const {
        if (unlikely(!is_some())) {
            PANIC_NSRC(std::forward<Args>(args) ...);
        }
        return get();
    }

    [[nodiscard]] __fast_inline constexpr const T 
    unwrap_or(const T choice) const {
        if(unlikely(exists_ == false)) return choice;
        return get();
    }


    __fast_inline constexpr void 
    unexpected() const {
        return;
    }


    template<typename FnSome, typename FnNone>
    constexpr auto match(FnSome&& some_fn, FnNone&& none_fn) const& {
        if (is_some()) {
            return std::invoke(std::forward<FnSome>(some_fn), get());
        } else {
            return std::invoke(std::forward<FnNone>(none_fn));
        }
    }

    // 函数式映射 (Monadic map)
    template<
        typename Fn,
        typename TIResult = tmp::functor_ret_t<Fn>
    >
    constexpr auto map(Fn&& fn) const& -> Option<TIResult> {
        if (is_some()) 
            return Some<TIResult>(std::forward<Fn>(fn)(get()));
        return None;
    }

    // 链式操作 (Monadic and_then)
    template<
        typename Fn, 
        typename TIResult = std::invoke_result_t<Fn, T>, 
        typename TData = std::conditional_t<
            is_option_v<TIResult>, 
            option_type_t<TIResult>, 
            TIResult
        >
    >

    constexpr auto and_then(Fn&& fn) const -> Option<TData>{
        if (is_some()){
            if constexpr(is_option_v<TIResult>){
                return std::forward<Fn>(fn)(get());
            }else{
                return Some<TData>(std::forward<Fn>(fn)(get()));
            }
        }
        return None;
    }

    // template<typename U, typename Fn>
    // friend auto operator|(const Option<U>& opt, Fn&& fn);

    template<typename Fn>
    __fast_inline constexpr 
    const Option<T> & inspect(Fn && fn) const {
        if (is_some()) std::forward<Fn>(fn)(unwrap());
        return *this;
    }

    template<typename Dummy = void>
    requires (is_option_v<T>)
    __fast_inline constexpr auto flatten() const {
        using RT = typename T::value_type;
        using Ret = Option<RT>;
        if (is_none()) return Ret(None);

        const auto & inner = unwrap();
        if(inner.is_some())
            return Ret(Some(inner.unwrap()));
        else
            return Ret(None);
    }
};

template<typename T>
class Option<T &>{
public:
    [[nodiscard]] constexpr 
    Option(const Some<T *> & something):
        pobj_(something.get()){;}
    [[nodiscard]] constexpr 
    Option(T * pobj):
        pobj_(pobj){;}

    [[nodiscard]] constexpr 
    Option(_None_t): pobj_(nullptr){;}

    [[nodiscard]] constexpr 
    bool is_some() const {return pobj_ != nullptr;}
    [[nodiscard]] constexpr 
    bool is_none() const {return pobj_ == nullptr;}
    
    [[nodiscard]] constexpr 
    T & unwrap() const {
        if(pobj_ == nullptr) [[unlikely]]
            {__builtin_abort();}
        return *pobj_;
    }

    template<
        typename Arg, 
        typename Ret = std::conditional_t<std::is_const_v<Arg>, const T &, T &>
    >
    [[nodiscard]] constexpr 
    Ret unwrap_or(Arg && other) const {
        if(pobj_ == nullptr) [[unlikely]]
            return other;
        return *pobj_;
    }
private:
    T * pobj_;
};

// 增强CTAD
template<typename T>
Option(Some<T>) -> Option<T>;

Option(_None_t) -> Option<std::monostate>;  // 显式空类型


template<typename T, typename Fn>
constexpr auto operator|(const Option<T>& opt, Fn&& fn){
    return (opt.and_then(std::forward<Fn>(fn)));
}

template<typename T, typename Fn>
constexpr auto operator|(Option<T> && opt, Fn&& fn){
    return (std::move(opt).and_then(std::forward<Fn>(fn)));
}

template<
    typename T, 
    typename TDecay = std::decay_t<T>
>
[[nodiscard]] constexpr Option<TDecay> optcond(bool cond, T&& value){
    if(cond) return Some<TDecay>(std::forward<TDecay>(value));
    else return None;
}

template<
    typename T, 
    typename TDecay = std::decay_t<T>
>
[[nodiscard]] constexpr Option<TDecay> optcond(bool cond, Some<TDecay>&& value){
    if(cond) return Some<TDecay>(std::forward<Some<TDecay>>(value));
    else return None;
}


template <typename T>
struct __unwrap_helper<Option<T>> {
    using Obj = Option<T>;
    // Unwrap a non-const rvalue Option
    static constexpr T unwrap(Obj && obj) {
        return std::move(obj.unwrap());
    }

    static constexpr T unwrap(const Obj & obj) {
        return obj.value();
    }

    static constexpr void unexpected(Obj && obj) {
        return std::move(obj.unexpected());
    }

    static constexpr void unexpected(const Obj & obj) {
        return obj.unexpected();
    }
};


template<typename T>
OutputStream & operator<<(OutputStream & os, const Option<T> & opt) {
    if(opt.is_some())
        return os << "Some" << os.brackets<'('>() << opt.unwrap() << os.brackets<')'>();
    else 
        return os << "None";
}
}


namespace std{
    // 结构化绑定支持 (C++17)
    template<typename T>
    struct tuple_size<ymd::Option<T>> 
        : integral_constant<std::size_t, 1> {};

    template<typename T>
    struct tuple_element<0, ymd::Option<T>> {
        using type = T;
    };

}