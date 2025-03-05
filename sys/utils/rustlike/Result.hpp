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
    struct _Storage_Diff{
    public:
        using Data = std::variant<T, E>;
        using ok_type = T;
        using err_type = E;
    
        __fast_inline constexpr _Storage_Diff(const Ok<T> & val):
            data_(T(val)){;}
    
    
        __fast_inline constexpr _Storage_Diff(const Err<E> & val):
            data_(E(val)){;}
    
        __fast_inline constexpr _Storage_Diff(const _Storage_Diff &) = default;
        __fast_inline constexpr _Storage_Diff(_Storage_Diff &&) = default;
    
        __fast_inline constexpr bool is_ok() const{return std::holds_alternative<T>(data_);}
        __fast_inline constexpr bool is_err() const{return std::holds_alternative<E>(data_);}
    
        __fast_inline constexpr T unwrap() const{return std::get<T>(data_);}
        __fast_inline constexpr E unwrap_err() const{return std::get<E>(data_);}
    private:
        Data data_;
    };
    
    template<typename T, typename E>
    struct _Storage_Uint{
    private:
        static constexpr size_t ok_size = sizeof(T);
        static constexpr size_t err_size = sizeof(E);
        static constexpr size_t max_size = std::max(ok_size,err_size);
        using data_type = size_to_int_t<max_size>;

    public:
        using ok_type = T;
        using err_type = E;
    
        __fast_inline constexpr _Storage_Uint(const Ok<T> & val):
            data_(static_cast<data_type>(T(val))){;}
        __fast_inline constexpr _Storage_Uint(const Err<E> & val):
            data_(static_cast<data_type>(- E(val)) - 1){;}
    
        __fast_inline constexpr _Storage_Uint(const _Storage_Uint &) = default;
        __fast_inline constexpr _Storage_Uint(_Storage_Uint &&) = default;
    
        __fast_inline constexpr bool is_ok() const{return data_ >= 0;}
        __fast_inline constexpr bool is_err() const{return data_ < 0;}
    
        __fast_inline constexpr T unwrap() const{return static_cast<T>(data_);}
        __fast_inline constexpr E unwrap_err() const{return static_cast<E>(- data_ - 1);}
    private:
        data_type data_;
    };

    template<typename T>
    struct _Storage_Same{
    public:
        using ok_type = T;
        using err_type = T;
        __fast_inline constexpr _Storage_Same(const Ok<T> & val):
            ok_data_(T(val)), is_ok_(true){;}    
        __fast_inline constexpr _Storage_Same(const Err<T> & val):
            err_data_(T(val)), is_ok_(false){;}
    
        __fast_inline constexpr _Storage_Same(const _Storage_Same &) = default;
        __fast_inline constexpr _Storage_Same(_Storage_Same &&) = default;
    
        __fast_inline constexpr bool is_ok() const{return is_ok_;}
        __fast_inline constexpr bool is_err() const{return !is_ok_;}
    
        __fast_inline constexpr T unwrap() const{return ok_data_;}
        __fast_inline constexpr T unwrap_err() const{return err_data_;}
    private:
        union{
            T ok_data_;
            T err_data_;
        };

        bool is_ok_;
    };
    
    template<typename E>
    struct _Storage_ErrorOnly{
        using ok_type = void;
        using err_type = E;
        using Data = std::optional<E>;
        __fast_inline constexpr _Storage_ErrorOnly(Ok<void> &&):
            data_(std::nullopt){;}
    
        __fast_inline constexpr _Storage_ErrorOnly(const Ok<void> &):
            data_(std::nullopt){;}
    
        __fast_inline constexpr _Storage_ErrorOnly(const Err<E> & val):
            data_(std::forward<E>(val)){;}
    
        __fast_inline constexpr _Storage_ErrorOnly(const _Storage_ErrorOnly &) = default;
        __fast_inline constexpr _Storage_ErrorOnly(_Storage_ErrorOnly &&) = default;
    
        __fast_inline constexpr bool is_ok() const{return !data_.has_value();}
        __fast_inline constexpr bool is_err() const{return data_.has_value();}
    
        __fast_inline constexpr void unwrap() const{}
        __fast_inline constexpr E unwrap_err() const{return (data_.value());}
    private:
        Data data_;
    };

    template<typename T>
    struct _Storage_OkOnly{
        using ok_type = T;
        using err_type = void;
        using Data = std::optional<T>;
        __fast_inline constexpr _Storage_OkOnly(const Ok<void> & val):
            data_(static_cast<T>(val)){;}
    
        __fast_inline constexpr _Storage_OkOnly(const Err<T> &):
            data_(std::nullopt){;}
    
        __fast_inline constexpr _Storage_OkOnly(const _Storage_OkOnly &) = default;
        __fast_inline constexpr _Storage_OkOnly(_Storage_OkOnly &&) = default;
    
        __fast_inline constexpr bool is_ok() const{return data_.has_value();}
        __fast_inline constexpr bool is_err() const{return !data_.has_value();}
    
        __fast_inline constexpr T unwrap() const{return (data_.value());}
        __fast_inline constexpr void unwrap_err() const{return;}
    private:
        Data data_;
    };
    
    struct _Storage_VoidOnly{
        using ok_type = void;
        using err_type = void;

        __fast_inline constexpr _Storage_VoidOnly(const Ok<void> &):
            is_ok_(true){}
        __fast_inline constexpr _Storage_VoidOnly(const Err<void> &):
            is_ok_(false){}
    
        __fast_inline constexpr _Storage_VoidOnly(const _Storage_VoidOnly &) = default;
        __fast_inline constexpr _Storage_VoidOnly(_Storage_VoidOnly &&) = default;
    
        __fast_inline constexpr bool is_ok() const{return is_ok_;}
        __fast_inline constexpr bool is_err() const{return !is_ok_;}
        __fast_inline constexpr void unwrap() const{}
        __fast_inline constexpr void unwrap_err() const{}
    private:
        bool is_ok_;
    };
    

    template<typename T>
    using storage_same_type_t = std::conditional_t<
        std::is_same_v<T, void>,
        _Storage_VoidOnly,
        _Storage_Same<T>
    >;

    template<typename T, typename E>
    using storage_diff_type_t = std::conditional_t<
        std::is_same_v<T, void>,
        _Storage_ErrorOnly<E>,
        std::conditional_t<
            std::is_same_v<E, void>,
            _Storage_OkOnly<T>,
            _Storage_Diff<T, E>
        >
    >;

    template<typename T, typename E>
    using storage_not_int_t = std::conditional_t<
        std::is_same_v<T, E>,
        storage_same_type_t<T>,
        storage_diff_type_t<T, E>
    >;

    template<typename T, typename E>
    using storage_t = std::conditional_t<
        (std::is_unsigned_v<T> && std::is_unsigned_v<E>),
        _Storage_Uint<T, E>,
        storage_not_int_t<T, E>
    >;
}



template<typename T, typename E>
class Result{
// private:
    // using Storage = std::variant<T, E>;
    // using __Result_details::operator|;
    // using _Storage = __Result_details::_Storage;
public:
    using Storage = __Result_details::storage_t<T, E>;
    using ok_type = Storage::ok_type;
    using err_type = Storage::err_type;
private:
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
    [[nodiscard]] __fast_inline constexpr Result(Ok<T> && value) : result_((value)){}
    [[nodiscard]] __fast_inline constexpr Result(const Ok<T> & value) : result_((value)){}

    [[nodiscard]] __fast_inline constexpr Result(Err<E> && unwrap_err) : result_((unwrap_err)){}
    [[nodiscard]] __fast_inline constexpr Result(const Err<E> & unwrap_err) : result_((unwrap_err)){}

    
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
    [[nodiscard]] __fast_inline constexpr auto map(F&& fn) const -> Result<TFReturn, E>{
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
    [[nodiscard]] __fast_inline constexpr auto and_then(F&& fn) const 
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


    template<
        typename Fn,//函数的类型
        typename E2
    >
    [[nodiscard]] __fast_inline constexpr auto validate(Fn&& fn, E2 && err) const 
        -> Result<T, E2>
    {
        
        if(is_ok()){
            if (false == std::forward<Fn>(fn)(unwrap())){
                return Err<E2>(std::forward<E2>(err));
            }else{
                return Ok<T>(unwrap());
            }
        }else{
            return Err<E2>(err);
        }
    }


    template<typename Fn>
    requires (!std::is_void_v<std::invoke_result_t<Fn, T>>)
    [[nodiscard]]__fast_inline constexpr auto transform(Fn&& fn) const & {
        // static_assert(>, "transform function must return a value");
        return and_then([fn=std::forward<Fn>(fn)](auto&& val){
            return fn(std::forward<decltype(val)>(val));
        });
    }

    // template<typename Fn>
    // __fast_inline constexpr auto transform(Fn&& fn) const & {
    //     return and_then([fn=std::forward<Fn>(fn)](auto&& val) -> Result<
    //         typename std::invoke_result_t<Fn, T>::ok_type,  // 推导新Ok类型
    //         typename std::invoke_result_t<Fn, T>::err_type  // 推导新Err类型
    //     > {
    //         return fn(std::forward<decltype(val)>(val));
    //     });
    // }

    template<typename Fn>
    __fast_inline constexpr void if_ok(Fn && fn) const {
        if (is_ok()) {
            std::forward<Fn>(fn)();
        }
    }

    template<typename Fn>
    __fast_inline constexpr void if_err(Fn && fn) const {
        if (is_err()) {
            std::forward<Fn>(fn)();
        }
    }


    [[nodiscard]] __fast_inline constexpr bool is_ok() const {
        return result_.is_ok();
    }

    [[nodiscard]] __fast_inline constexpr bool is_err() const {
        return result_.is_err();
    }
    


    template<typename ... Args>
    constexpr T expect(Args && ... args) const{
        if (likely(is_ok())) {
            return result_.unwrap();
        } else {
            #ifdef __DEBUG_INCLUDED
            PANIC_NSRC(std::forward<Args>(args)...);
            #endif
            exit(1);
        }
    }
    
    template<typename ... Args>
    const Result & check(Args && ... args) const{
        if(unlikely(is_err())){
            #ifdef __DEBUG_INCLUDED
            DEBUG_PRINTLN(unwrap_err(), std::forward<Args>(args)...);
            #endif
        }
        return *this;
    } 

    [[nodiscard]] constexpr _Loc loc(const std::source_location & loca = std::source_location::current()) const{
        return {*this, loca};
    }

    __fast_inline constexpr T unwrap() const {
        if (likely(is_ok())) {
            return result_.unwrap();
        } else {
            exit(1);
        }
    }

    __fast_inline constexpr T operator +() const{
        if(likely(is_ok())){
            return result_.unwrap();
        }else{
            exit(1);
        }
    }

    __fast_inline constexpr E unwrap_err() const {
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

    constexpr bool contains(auto && val){
        return is_ok() ? (unwrap() == static_cast<T>(val)) : false;
    }

    template<
        typename FnOk,
        typename FnErr,
        typename TOkReturn = std::invoke_result_t<FnOk, T>,//函数返回值的类型
        typename TErrReturn = std::invoke_result_t<FnErr, E>//函数返回值的类型
    >
    __fast_inline constexpr auto match(FnOk && fn_ok, FnErr && fn_err) const 
    -> Result<TOkReturn, TErrReturn>{
        if(is_ok()) return Ok<TOkReturn>(std::forward<FnOk>(fn_ok)(unwrap()));
        return Ok<TOkReturn>(std::forward<FnOk>(fn_ok)(unwrap()));
    }

    template<typename U, typename Fn>
    friend auto __Result_details::operator|(U&& val, Fn&& fn);

        // 添加隐式类型转换运算符
    template<typename U, typename V>
    operator Result<U, V>() const {
        if (is_ok()) {
            return Ok<U>(unwrap());
        } else {
            return Err<V>(unwrap_err());
        }
    }
};

template<
    typename T, 
    typename E,
    typename Tdecay = std::decay_t<T>,
    typename Edecay = std::decay_t<E>
>
[[nodiscard]] Result<Tdecay, Edecay> rescond(bool cond, T&& ok, E&& err){
    if(cond) return Ok<Tdecay>(std::forward<T>(ok));
    else return Err<Edecay>(std::forward<E>(err));
}

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

template<typename TDummy = void>
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