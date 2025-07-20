#pragma once

#include "core/math/real.hpp"
#include "core/math/iq/iq_t.hpp"

namespace ymd{

template<arithmetic T>
requires (is_fixed_point_v<T>)
static constexpr size_t iq_num_v = T::q_num;


template<typename T>
class Norm_t{
public:
    using type = T;

    __fast_inline constexpr
    explicit Norm_t(const T & value):
        value_(value){}

    __fast_inline constexpr
    Norm_t(): value_(T{}){}

    constexpr
    Norm_t(const Norm_t<T> & other):
    value_(other.value_){;}
    
    constexpr
    Norm_t(Norm_t<T> && other):
        value_(other.value_){;}

    constexpr
    Norm_t & operator = (const Norm_t<T> & other){value_ = other.value_; return *this;}
    
    constexpr
    Norm_t & operator = (Norm_t<T> && other){value_ = other.value_; return*this;}

    constexpr
    T operator + (const Norm_t<T> & other) const {
        return value_ + other.value_;
    }

    constexpr
    T operator - (const Norm_t<T> & other) const {
        return value_ - other.value_;
    }

    constexpr
    Norm_t<T> operator + () const{
        return Norm_t(value_);
    }

    constexpr
    Norm_t<T> operator - () const {
        return Norm_t(- value_);
    }

    __fast_inline constexpr
    operator T() const{
        return value_;
    }
private:    
    T value_;
};

template<arithmetic T>
constexpr
T operator-(const T lhs, const Norm_t<T> & rhs){
    return lhs - static_cast<T>(rhs);
}

template<arithmetic T>
constexpr
T operator+(const T lhs, const Norm_t<T> & rhs){
    return lhs + static_cast<T>(rhs);
}

template<arithmetic T>
__fast_inline constexpr
T operator*(const T & lhs, const Norm_t<T> & rhs){
    return lhs * static_cast<T>(rhs);
}

template<arithmetic T>
__fast_inline constexpr
Norm_t<T> operator*(const Norm_t<T> & lhs, const Norm_t<T> & rhs){
    // return qmux(static_cast<T>(lhs), static_cast<T>(rhs));
    return qmux(lhs, rhs);
}





//CTAD
template<arithmetic T>
Norm_t() -> Norm_t<T>;



//#region qmux

//为了加速标幺数的计算 提供了快速卷积


//常规实现
template<arithmetic T>
__inline constexpr
T qmux(const T a, const T b){
    return a * b;
}

//已归一化的定点数的实现
//若已经归一化 则当两个定点数的乘法满足不会溢出时 不使用64位乘法扩张 而是直接进行32位运算
template<arithmetic T>
requires (ymd::is_fixed_point_v<T>)
__inline constexpr
Norm_t<T> qmux(const Norm_t<T> a, const Norm_t<T> b){
    if constexpr (ymd::is_fixed_point_v<T> and ymd::iq_num_v<T> < 15){
        constexpr size_t q_num = ymd::iq_num_v<T>;
        return Norm_t(iq_t<q_num>(std::bit_cast<_iq<q_num>>(
            (T(a).as_i32() * T(b).as_i32()) >> q_num)
        ));
    }else{
        return Norm_t(T(a) * T(b));
    }
}

//#endregion

namespace details{

template<arithmetic T>
__inline constexpr
Norm_t<T> _qconv_norm(){
    return Notm_t(T(0));
}


template<arithmetic T>
__inline constexpr
Norm_t<T> _qconv_norm(Norm_t<T> const & a){
    return a;
}

template<arithmetic T>
__inline constexpr
Norm_t<T> _qconv_norm(Norm_t<T> const & a, Norm_t<T> const & b){
    return qmux(a, b);
}

template<arithmetic T, typename ... Args>
__inline constexpr
T _qconv_norm(Norm_t<T> const & a, Norm_t<T> const & b, Args const & ... args){
    if constexpr (sizeof...(Args)) return T(_qconv_norm(a, b)) + T(_qconv_norm(args...));
    else return _qconv_norm(a,b) + _qconv_norm(args...);
}
}


template<arithmetic T, typename ... Args>
__inline constexpr
T qconv(Norm_t<T> const & a, Norm_t<T> const & b, Args const & ... args){
    return details::_qconv_norm(a,b, args...);
}

template<arithmetic T, typename ... Args>
__inline constexpr
T qconv(T const & a, T const & b, Args const & ... args){
    return details::_qconv_norm(a,b, args...);
}


template<arithmetic T>
__inline constexpr
Norm_t<T> qsqu(Norm_t<T> const & x){
    return qmux(x,x);
}

}

namespace std{
    //使得Norm_t的属性与内部类型保持一致

    template<typename T>
    struct is_arithmetic<ymd::Norm_t<T>> : std::conditional_t<
        std::is_arithmetic_v<T>, std::true_type, std::false_type>{};
    
    template<typename T>
    struct is_floating_point<ymd::Norm_t<T>> : std::conditional_t<
        std::is_floating_point_v<T>, std::true_type, std::false_type>{};
}
