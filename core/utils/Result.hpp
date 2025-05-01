#pragma once

#include "Option.hpp"
#include "typetraits/size_traits.hpp"


namespace ymd{

namespace custom{
    template <typename T, typename E, typename S> 
    struct result_converter{};



    // 非侵入式地添加隐式类型转换
    // T为正确类型 E为错误类型 S为源类型

    // eg:
    // template<>
    // struct result_converter<void, hal::HalResult, hal::HalResult> {
    //     static Result<void, hal::HalResult> convert(const hal::HalResult & res){
    //         if(res.ok()) return Ok();
    //         else return Err(res); 
    //     }
    // };

}

template<typename T>
static constexpr bool is_result_v = false;

template<typename T, typename E>
static constexpr bool is_result_v<Result<T, E>> = true;

template<typename T>
using is_result_t = std::conditional_t<is_result_v<T>, std::true_type, std::false_type>;


namespace details{
template<typename T>
struct _result_type{
};

template<typename T, typename E>
struct _result_type<Result<T, E>>{
    using ok_type = T;
    using err_type = E;
};

}

template<typename TResult>
using result_ok_type_t = details::_result_type<TResult>::ok_type;

template<typename TResult>
using result_err_type_t = details::_result_type<TResult>::err_type;


namespace details{
    template<typename T, typename E>
    struct _Storage_Diff{
    public:
        using Data = std::variant<T, E>;
        using ok_type = T;
        using err_type = E;
    
        __fast_inline constexpr _Storage_Diff(const Ok<T> & val):data_(val.unwrap()){;}
        __fast_inline constexpr _Storage_Diff(Ok<T> && val):data_(std::move(val.unwrap())){;}
    
    
        __fast_inline constexpr _Storage_Diff(const Err<E> & val):data_(val.unwrap()){;}
        __fast_inline constexpr _Storage_Diff(Err<E> && val):data_(std::move(val.unwrap())){;}
    
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
            data_(val.unwrap()){;}
    
        __fast_inline constexpr _Storage_ErrorOnly(const _Storage_ErrorOnly &) = default;
        // __fast_inline constexpr _Storage_ErrorOnly(_Storage_ErrorOnly &&) = default;
    
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
        __fast_inline constexpr _Storage_OkOnly(const Ok<T> & val):
            data_(val.unwrap()){;}
    
        __fast_inline constexpr _Storage_OkOnly(const Err<void> &):
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


template <typename T, typename E>
class Result{
public:
    using Storage = details::storage_t<T, E>;
    using ok_type = Storage::ok_type;
    using err_type = Storage::err_type;
    using type = Result<T, E>;
    constexpr Result & operator =(const Result<T, E> &) = default;

private:
    Storage storage_;

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
            return storage_.unwrap();
        } else {
            #ifdef __DEBUG_INCLUDED
            __PANIC_EXPLICIT_SOURCE(loc, std::forward<Args>(args)...);
            #endif
            __builtin_abort();
        }
    }
    friend class _Loc;
public:

    template<typename T2>
    requires ((not std::is_void_v<T2>) and std::is_convertible_v<Ok<T2>, Ok<T>>)
    [[nodiscard]] __fast_inline constexpr Result(const Ok<T2> & value) : storage_(value){}

    template<typename T2 = T>
    requires (std::is_void_v<T2>)
    [[nodiscard]] __fast_inline constexpr Result(const Ok<T2> & value) : storage_(Ok<void>()){}
    
    template<typename E2>
    requires ((not std::is_void_v<E2>) and std::is_convertible_v<Err<E2>, Err<E>>)
    [[nodiscard]] __fast_inline constexpr Result(const Err<E2> & error) : storage_(error){}

    template<typename E2 = E>
    requires (std::is_void_v<E2>)

    [[nodiscard]] __fast_inline constexpr Result(const Err<E2> & value) : storage_(Err<void>()){}


    template<typename S>
    requires requires(S s) {
        { custom::result_converter<T, E, S>::convert(s) } -> std::convertible_to<Result<T, E>>;
    }
    [[nodiscard]] __fast_inline constexpr explicit 
    Result(const S & other):Result(custom::result_converter<T, E, S>::convert(other)){}


    template<typename T2>
    [[nodiscard]] __fast_inline constexpr
    Result<T2, E> operator |(Result<T2, E> && rhs) const {
        if(is_ok()) return rhs;
        else return Err(this->unwrap_err());
    }

    template<typename T2>
    [[nodiscard]] __fast_inline constexpr
    Result<T2, E> operator |(const Result<T2, E> & rhs) const {
        if(is_ok()) return rhs;
        else return Err(this->unwrap_err());
    }

    template<
        typename Fn,
        typename FDecay = std::decay_t<Fn>,

        typename TFReturn = std::invoke_result_t<FDecay, T>
    >
    [[nodiscard]] __fast_inline constexpr auto map(Fn && fn) const -> Result<TFReturn, E>{
        if (is_ok()) {
            if constexpr(std::is_void_v<TFReturn>) return Ok<TFReturn>(std::forward<Fn>(fn)());
            else return Ok<TFReturn>(std::forward<Fn>(fn)(storage_.unwrap()));
        }
        else return Err<E>(unwrap_err());
    }

    template<typename Tok>
    [[nodiscard]] __fast_inline constexpr 
    Result<std::decay_t<Tok>, E> to(Tok && ok) const{
        if (is_ok()) {
            return Ok<std::decay_t<Tok>>(ok);
        }
        else return Err<E>(unwrap_err());
    }
    

    template<
        typename F,//函数的类型
        // typename TFReturn = std::conditional_t<std::is_void_v<T>, std::invoke_result_t<typename F::operator()>, std::invoke_result_t<F>>,//函数返回值的类型
        typename TFReturn = std::invoke_result_t<F, T>,//函数返回值的类型
        typename TFReturnIsResult = is_result_t<TFReturn>,
        typename TOk = std::conditional_t<
            is_result_v<TFReturn>,
            result_ok_type_t<TFReturn>, 
            TFReturn
        >
        // 如果返回值本身是Result 那么返回它的解包类型，否则返回原类型
    >
    [[nodiscard]] __fast_inline constexpr 
    auto and_then(F&& fn) const 
        -> Result<TOk, E>
    {
        if (is_ok()){
            if constexpr (is_result_v<TFReturn>){
                if constexpr(std::is_void_v<T>) return (std::forward<F>(fn)());
                else return (std::forward<F>(fn)(unwrap()));
            }else{
                if constexpr(std::is_void_v<T>)return Ok<TOk>(std::forward<F>(fn)());
                else return Ok<TOk>(std::forward<F>(fn)(unwrap()));
            }
        }
        return Err<E>(unwrap_err());
    }

    template<typename Fn>
    [[nodiscard]] __fast_inline constexpr 
    auto then(Fn && fn) const 
        -> Result<T, E>
    {
        if (is_ok()){
            return (std::forward<Fn>(fn)());
        }
        return Err<E>(unwrap_err());
    }


    template<
        typename Fn,//函数的类型
        typename E2
    >
    [[nodiscard]] __fast_inline constexpr 
    auto validate(Fn&& fn, E2 && err) const 
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

    template<
        typename ERaw,
        typename E2 = std::conditional_t<
            std::is_convertible_v<ERaw, E>,
            E, ERaw
            >
    >
    [[nodiscard]] __fast_inline constexpr 
    auto validate(bool valid, ERaw && err) const 
        -> Result<T, E2>
    {
        
        if(is_ok()){
            if (false == valid){
                return Err<E2>(std::forward<E2>(err));
            }else{
                if constexpr(std::is_void_v<T>) return Ok();
                else return Ok<T>(unwrap());
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
    __fast_inline constexpr 
    Result<T, E> inspect(Fn && fn) const {
        if (is_ok()){
            if constexpr(!std::is_void_v<T>) std::forward<Fn>(fn)(unwrap());
            else std::forward<Fn>(fn)();
        }
        return *this;
    }



    template<typename Fn>
    __fast_inline constexpr 
    const Result<T, E> & inspect_err(Fn && fn) const {
        if (is_err()) {
            std::forward<Fn>(fn)(unwrap_err());
        }
        return *this;
    }

    template<typename Fn>
    __fast_inline constexpr 
    Result<T, E> if_ok(Fn && fn) const {
        if (is_ok()) std::forward<Fn>(fn)();
        return *this;
    }

    [[nodiscard]] __fast_inline constexpr 
    bool is_ok() const {
        return storage_.is_ok();
    }

    [[nodiscard]] __fast_inline constexpr 
    bool is_err() const {
        return storage_.is_err();
    }
    


    template<typename ... Args>
    constexpr 
    T expect(Args && ... args) const{
        if (likely(is_ok())) {
            return storage_.unwrap();
        } else {
            #ifdef __DEBUG_INCLUDED
            PANIC_NSRC(std::forward<Args>(args)...);
            #endif
            __builtin_abort();
        }
    }
    
    template<typename ... Args>
    [[nodiscard]] constexpr
    const Result & check(Args && ... args) const{
        if(unlikely(is_err())){
            #ifdef __DEBUG_INCLUDED
            DEBUG_PRINTLN(unwrap_err(), std::forward<Args>(args)...);
            #endif
        }
        return *this;
    } 
    
    template<bool en, typename ... Args>
    [[nodiscard]] constexpr 
    const Result & check_if(Args && ... args) const{
        if constexpr (en) {
            return check(std::forward<Args>(args)...);
        }
        return *this;
    }
    [[nodiscard]] constexpr 
    _Loc loc(const std::source_location & loca = std::source_location::current()) const{
        return {*this, loca};
    }

    __fast_inline constexpr 
    T unwrap() const {
        if (likely(is_ok())) {
            return storage_.unwrap();
        } else {
            __builtin_abort();
        }
    }

    
    __fast_inline constexpr 
    T unwrap_or(auto && val) const {
        if (likely(is_ok())) {
            return storage_.unwrap();
        } else {
            return static_cast<T>(val);
        }
    }

    

    struct _PanicWithOutUnlock{
        ~_PanicWithOutUnlock(){
            if (will_panic) {
                PANIC("unsafe ignore without unlock");
            }
        }

        void operator !(){
            will_panic = false;
        }
    private:
        bool will_panic = true;
    };

    __fast_inline constexpr 
    auto operator +() const{
        return _PanicWithOutUnlock{};
    }

    __fast_inline constexpr 
    T operator !() const{
        return storage_.unwrap();
    }

    __fast_inline constexpr 
    E unwrap_err() const {
        if (likely(is_err())) {
            return storage_.unwrap_err();
        } else {
            __builtin_abort();
        }
    }

    constexpr Option<T> 
    ok() const{
        if (likely(is_ok())) {
            return Some(storage_.unwrap());
        } else {
            return None;
        }
    }

    constexpr Option<E> 
    err() const{
        if (likely(is_err())) {
            return Some(storage_.unwrap_err());
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

template<
    typename T, 
    typename E,
    typename Tdecay = std::decay_t<T>,
    typename Edecay = std::decay_t<E>
>
[[nodiscard]] Result<Tdecay, Edecay> rescond(bool cond, Ok<T>&& ok, Err<E>&& err){
    if(cond) return Ok<Tdecay>((ok));
    else return Err<Edecay>((err));
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