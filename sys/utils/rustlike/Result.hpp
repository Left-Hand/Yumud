#pragma once

#include "Optional.hpp"

namespace ymd{


template<typename T>
static constexpr bool is_result_v = false;

template<typename T, typename E>
static constexpr bool is_result_v<Result<T, E>> = true;

template<typename T>
using is_result_t = std::conditional_t<
    is_result_v<T>, 
    std::true_type, 
    std::false_type
>;


template<typename T>
struct _result_type{
};

template<typename T, typename E>
struct _result_type<Result<T, E>>{
    using ok_type = T;
    using err_type = E;
};

template<typename TResult>
using result_ok_type_t = _result_type<TResult>::ok_type;

template<typename TResult>
using result_err_type_t = _result_type<TResult>::err_type;


namespace __Result_details{
    template<typename U, typename Fn>
    auto operator|(U&& val, Fn&& fn) {
        return std::forward<Fn>(fn)(std::forward<U>(val));
    }


    template<typename T, typename E>
    struct _Storage{
    public:
        using Data = std::variant<T, E>;
    
        constexpr _Storage(const Ok<T> & val):
            data_(T(val)){;}
    
        // constexpr _Storage(T && val):
        //     data_(std::move(val)){;}
    
        constexpr _Storage(const Err<E> & val):
            data_(E(val)){;}
    
        // constexpr _Storage(E && val):
        //     data_(std::move(val)){;}
    
        constexpr _Storage(const _Storage &) = default;
        constexpr _Storage(_Storage &&) = default;
    
        constexpr bool is_ok() const{return std::holds_alternative<T>(data_);}
        constexpr bool is_err() const{return std::holds_alternative<E>(data_);}
    
        constexpr T unwrap() const{return std::get<T>(data_);}
        constexpr E unwrap_err() const{return std::get<E>(data_);}
    private:
        Data data_;
    };
    
    template<typename E>
    struct _Storage<void, E>{
        using Data = std::optional<E>;
        constexpr _Storage(Ok<void> &&):
            data_(std::nullopt){;}
    
        constexpr _Storage(const Ok<void> &):
            data_(std::nullopt){;}
    
        constexpr _Storage(const Err<E> & val):
            data_(std::forward<E>(val)){;}
    
        constexpr _Storage(const _Storage &) = default;
        constexpr _Storage(_Storage &&) = default;
    
        constexpr bool is_ok() const{return !data_.has_value();}
        constexpr bool is_err() const{return data_.has_value();}
    
        constexpr void unwrap() const{}
        constexpr E unwrap_err() const{return (data_.value());}
    private:
        Data data_;
    };
    
    template<>
    struct _Storage<void, void>{
        constexpr _Storage(Ok<void> &&){}
    
        constexpr _Storage(const Ok<void> &){}
    
        constexpr _Storage(const Err<void> & val){}
    
        constexpr _Storage(const _Storage &) = default;
        constexpr _Storage(_Storage &&) = default;
    
        constexpr bool is_ok() const{return is_ok_;}
        constexpr bool is_err() const{return !is_ok_;}
        constexpr void unwrap() const{}
        constexpr void unwrap_err() const{}
    private:
        bool is_ok_;
    };
    
}



template<typename T, typename E>
class Result{
private:
    // using Storage = std::variant<T, E>;
    // using __Result_details::operator|;
    // using _Storage = __Result_details::_Storage;

    using Storage = __Result_details::_Storage<T, E>;

    Storage result_;

    struct _Loc{
        const Result<T, E> & owner_;
        std::source_location loc_;

        template<typename ... Args>
        constexpr T expect(Args && ... args) const{
            return owner_.except_impl(loc_, std::forward<Args>(args)...);
        }
    };

    template<typename ... Args>
    constexpr T except_impl(
        const std::source_location & loc,
        Args && ... args
    ) const {
        if (likely(is_ok())) {
            return result_.unwrap();
        } else {
            #ifdef __DEBUG_INCLUDED
            __PANIC_EXPLICIT_SOURCE(loc, std::forward<Args>(args)...);
            #endif
            exit(1);
        }
    }
    friend class _Loc;
public:
    // template<typename TDummy = void>
    // requires (!std::is_void_v<T>)
    // constexpr Result(T value) : result_(std::move(value)) {}

    // template<typename TDummy = void>
    // requires (std::is_void_v<T>)
    // constexpr Result(void){}

    // constexpr Result(E unwrap_err) : result_(std::move(unwrap_err)) {}
    

    // template<typename T>
    constexpr Result(Ok<T> && value) : result_((value)){}
    constexpr Result(const Ok<T> & value) : result_((value)){}

    constexpr Result(Err<E> && unwrap_err) : result_((unwrap_err)){}
    constexpr Result(const Err<E> & unwrap_err) : result_((unwrap_err)){}

    
    // // 映射成功值
    // template<typename F>
    // constexpr auto map(F&& fn) -> Result<std::invoke_result_t<F, T>, E> const {
    //     if (is_ok()) return fn(unwrap());
    //     else return unwrap_err();
    // }

    // // 链式处理
    // template<typename F>
    // constexpr auto and_then(F&& fn) -> Result<std::invoke_result_t<F, T>, E> const {
    //     if (is_ok()) return fn(unwrap());
    //     else return unwrap_err();
    // }

        // 修改map方法
    template<
        typename F,
        typename TFReturn = std::invoke_result_t<F, T>
    >
    constexpr auto map(F&& fn) const -> Result<TFReturn, E>{
        if (is_ok()) return Ok<TFReturn>(std::forward<F>(fn)(result_.unwrap()));
        else return Err<E>(unwrap_err());
    }
    
    // 修改and_then方法
    template<
        typename F,//函数的类型
        typename TFReturn = std::invoke_result_t<F, T>,//函数返回值的类型
        typename TFReturnIsResult = is_result_t<TFReturn>,
        typename TOk = std::conditional_t<
            is_result_v<TFReturn>,
            result_ok_type_t<TFReturn>, 
            TFReturn
        >
        // 如果返回值本身是Result 那么返回它的解包类型，否则返回原类型
    >
    constexpr auto and_then(F&& fn) const 
        -> Result<TOk, E>
    {
        if (is_ok()){
            if constexpr (is_result_v<TFReturn>){
                return (std::forward<F>(fn)(unwrap()));
            }else{
                return Ok<TOk>(std::forward<F>(fn)(unwrap()));
            }
        }
        return Err<E>(unwrap_err());
    }

    template<typename F>
    constexpr void if_ok(F&& fn) const {
        if (is_ok()) {
            (fn)();
        }
    }


    constexpr bool is_ok() const {
        return result_.is_ok();
    }

    constexpr bool is_err() const {
        return result_.is_err();
    }
    


    template<typename ... Args>
    constexpr T expect(Args && ... args) const{
        if (likely(is_ok())) {
            return result_.unwrap();
        } else {
            #ifdef __DEBUG_INCLUDED
            DEBUG_PRINTS(std::forward<Args>(args)...);
            #endif
            exit(1);
        }
    }
    
    template<typename ... Args>
    const Result & check(Args && ... args) const{
        if(unlikely(is_err())){
            #ifdef __DEBUG_INCLUDED
            DEBUG_PRINTS(unwrap_err(), std::forward<Args>(args)...);
            #endif
            exit(1);
        }
        return *this;
    } 

    constexpr _Loc loc(const std::source_location & loca = std::source_location::current()) const{
        return {*this, loca};
    }

    constexpr T unwrap() const {
        if (likely(is_ok())) {
            return result_.unwrap();
        } else {
            exit(1);
        }
    }

    constexpr E unwrap_err() const {
        if (likely(is_err())) {
            return result_.unwrap_err();
        } else {
            exit(1);
        }
    }

    constexpr Option<T> ok() const{
        if (likely(is_ok())) {
            return Some(result_.unwrap());
        } else {
            return None;
        }
    }

    constexpr Option<E> err() const{
        if (likely(is_err())) {
            return Some(result_.unwrap_err());
        } else {
            return None;
        }
    }

    template<typename U, typename Fn>
    friend auto __Result_details::operator|(U&& val, Fn&& fn);
};


// 特化版本处理Result类型
template<typename T, typename E, typename Fn>
auto operator|(const Result<T, E>& res, Fn&& fn) {
    return res.and_then(std::forward<Fn>(fn));
}

template<typename T, typename E, typename Fn>
auto operator|(Result<T, E>&& res, Fn&& fn) {
    return std::move(res).and_then(std::forward<Fn>(fn));
}

template<typename E>
Result(Err<E> && val) -> Result<void, E>;

template<typename TDummy>
Result() -> Result<void, void>;



// Specialization for std::optional
template <typename T, typename E>
struct __unwrap_helper<Result<T, E>> {
    using Obj = Result<T, E>;
    // Unwrap a non-const rvalue optional
    static constexpr bool is_ok(const Obj & obj) {
        return obj.is_ok();
    }
    static constexpr T && unwrap(Obj && obj) {
        return std::move(obj.unwrap());
    }
    
    static constexpr T unwrap(const Obj & obj) {
        return obj.unwrap();
    }
    
    static constexpr E unexpected(Obj && obj) {
        return std::move(obj.unwrap_err());
    }

    static constexpr E unexpected(const Obj & obj) {
        return obj.unwrap_err();
    }
};


}