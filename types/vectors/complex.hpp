#pragma once

#include "core/math/realmath.hpp"
#include "primitive/arithmetic/angle.hpp"
#include "types/vectors/Vector2.hpp"

namespace ymd{
template <typename T>
struct [[nodiscard]] UnitComplex {
private:
    
    __fast_inline constexpr UnitComplex(const T _re, const T _im)
        : re(_re), im(_im) {;}

public:
    using Self = UnitComplex;

    T re;
    T im;


    __fast_inline static constexpr UnitComplex from_uninitialized(){
        return UnitComplex();
    }


    [[nodiscard]] __fast_inline static constexpr 
    UnitComplex from_matrix(const Matrix<T, 2, 2> & m){
        return UnitComplex(m.template at<0,0>(), m.template at<0,1>());
    }

    [[nodiscard]] __fast_inline static constexpr 
    UnitComplex from_imag(const T val) {return UnitComplex(T(0), val);}

    [[nodiscard]] __fast_inline static constexpr 
    UnitComplex from_angle(const Angle<T> angle) {
        const auto [s,c] = angle.sincos();
        return UnitComplex(c, s);
    }

    [[nodiscard]] __fast_inline static constexpr 
    UnitComplex from_radians(const T radians) {
        const auto [s,c] = ymd::sincos(radians);
        return UnitComplex(c, s);
    }

    [[nodiscard]] constexpr 
    UnitComplex forward_90deg() const {
        return UnitComplex{-sine(), cosine()};
    }

    [[nodiscard]] constexpr
    UnitComplex backward_90deg() const {
        return UnitComplex{sine(), -cosine()};
    }

    [[nodiscard]] constexpr 
    Vec2<T> operator*(const Vec2<T>& v) const {
        return Vec2<T>(
            cosine() * v.x - sine() * v.y,
            sine() * v.x + cosine() * v.y
        );
    }

    [[nodiscard]] constexpr 
    UnitComplex<T> operator*(const UnitComplex<T> & other) const {
        // 三角函数公式：sin(a+b) = sin(a)cos(b) + cos(a)sin(b)
        //             cos(a+b) = cos(a)cos(b) - sin(a)sin(b)
        return UnitComplex<T>(
            cosine() * other.cosine() - sine() * other.sine(),
            sine() * other.cosine() + cosine() * other.sine()
        );
    }


    [[nodiscard]] constexpr T sine() const {return im;}
    [[nodiscard]] constexpr T cosine() const {return re;}

    [[nodiscard]] constexpr std::array<T, 2> sincos() const {
        return {sine(), cosine()};
    }

    [[nodiscard]] __fast_inline constexpr T arg() const {return std::atan2(im, re);}

    __fast_inline constexpr UnitComplex operator-() {
        return UnitComplex(-re, -im);
    }

    template<size_t I>
    [[nodiscard]] __fast_inline constexpr T & get(){
        return get_element(*this);
    }

    template<size_t I>
    [[nodiscard]] __fast_inline constexpr T get() const {
        return get_element(*this);
    }

    [[nodiscard]] constexpr Angle<T> to_angle() const {
        return Angle<T>::from_turns(atan2pu(im, re));
    }

    [[nodiscard]] constexpr Vec2<T> to_vec2(const T length) const {
        return Vec2<T>(re * length, im * length);
    }

    [[nodiscard]] constexpr Vec2<T> to_unit_vec2() const {
        return Vec2<T>(re, im);
    }


private:
    constexpr UnitComplex() = default;

    template<size_t I>
    [[nodiscard]] static constexpr auto & get_element(auto & self){
        if constexpr(I == 0){ return self.re; }
        else if constexpr(I == 1){ return self.im; }
    }

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os    
            << os.field("re")(self.re) << os.splitter()
            << os.field("im")(self.im)
        ;
    }
};

template<typename T>
struct [[nodiscard]] Complex{
public:
    using Self = Complex;
    T re;
    T im;

    __fast_inline static constexpr Complex from_uninitialized(){
        return Complex();
    }

    __fast_inline static constexpr Complex zero() {return Complex(T(0), T(0));} 
    
    __fast_inline static constexpr 
    Complex from_imag(const T val) {return Complex(T(0), val);}


    // __fast_inline constexpr UnitComplex<T> normalized(){
    //     const auto ret = *this;
    //     ret /= length(); 
    //     return {ret;
    // }

    [[nodiscard]] __fast_inline constexpr T real_squared() const {return re * re;}
    [[nodiscard]] __fast_inline constexpr T imag_squared() const {return im * im;}

    [[nodiscard]] __fast_inline constexpr T length() const {
        return mag(re, im);
    }

    [[nodiscard]] __fast_inline constexpr T inv_length() const {
        return im(re, im);
    }


    [[nodiscard]] constexpr Angle<T> to_angle() const {
        return Angle<T>::from_turns(atan2pu(im, re));
    }

    template <typename U>
    __fast_inline constexpr Complex& operator=(const Complex<U>& other) {
        if (this != &other) {
            re = static_cast<T>(other.re);
            im = static_cast<T>(other.im);
        }
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator+=(const Complex<U>& other) {
        re += static_cast<T>(other.re);
        im += static_cast<T>(other.im);
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator+=(const U & other) {
        re += static_cast<T>(other);
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator-=(const Complex<U>& other) {
        re -= static_cast<T>(other.re);
        im -= static_cast<T>(other.im);
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator-=(const U & other) {
        re -= static_cast<T>(other);
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator*=(const U & other) {
        re *= other;
        im *= other;
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator/=(const U & other) {
        re /= other;
        im /= other;
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator*=(const Complex<U>& other) {
        const auto p_real = re;
        const auto p_imag = im;
        re = static_cast<T>(static_cast<U>(p_real) * other.re - static_cast<U>(p_imag) * other.im);
        im = static_cast<T>(static_cast<U>(p_real) * other.im + static_cast<U>(p_imag) * other.re);
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator/=(const Complex<U>& other) {
        U denominator = other.norm();
        const auto p_real = re;
        const auto p_imag = im;
        re = static_cast<T>((static_cast<U>(p_real) * other.re + static_cast<U>(p_imag) * other.im) / denominator);
        im = static_cast<T>((static_cast<U>(p_imag) * other.re - static_cast<U>(p_real) * other.im) / denominator);
        return *this;
    }


private:

    constexpr Complex() = default;

    template<size_t I>
    [[nodiscard]] static constexpr auto & get_element(auto & self){
        if constexpr(I == 0){ return self.re; }
        else if constexpr(I == 1){ return self.im; }
    }

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os    
            << os.field("re")(self.re) << os.splitter()
            << os.field("im")(self.im)
        ;
    }
};
#define COMPLEX_ARITHMETIC_OPERATOR(op)\
\
template <typename T, typename U>\
__fast_inline Complex<T> operator op (Complex<T> lhs, const Complex<U>& rhs) {\
    lhs op##= rhs;\
    return lhs;\
}\
\
template <typename T, typename U>\
__fast_inline Complex<T> operator op (Complex<T> lhs, const U & rhs) {\
    lhs op##= static_cast<T>(rhs);\
    return lhs;\
}\
\
template <typename T, typename U>\
__fast_inline Complex<T> operator op (const U & lhs, Complex<T> rhs) {\
    Complex<T>(static_cast<T>(lhs), T(0)) op##= rhs;\
    return lhs;\
}

COMPLEX_ARITHMETIC_OPERATOR(+)
COMPLEX_ARITHMETIC_OPERATOR(-)
COMPLEX_ARITHMETIC_OPERATOR(*)
COMPLEX_ARITHMETIC_OPERATOR(/)


template <typename T, typename U>
__fast_inline bool operator==(const Complex<T>& lhs, const Complex<U>& rhs) {
    return lhs.re == static_cast<T>(rhs.re) && lhs.im == static_cast<T>(rhs.im);
}

template <typename T>
__fast_inline bool operator!=(const Complex<T>& lhs, const Complex<T>& rhs) {
    return !(lhs == rhs);
}

#define DEF_COMPLEX_COMPARE_IM_OPERATOR(op) \
\
template <typename T, typename U> \
__fast_inline bool operator op (const Complex<T>& lhs, const U& rhs) { \
    T absrhs = static_cast<T>(length(rhs)); \
    return lhs.norm() op (absrhs * absrhs); \
} \
\
template <typename T, typename U> \
__fast_inline bool operator op (const U& lhs, const Complex<T>& rhs) { \
    T abslhs = static_cast<T>(length(lhs)); \
    return (abslhs * abslhs) op (rhs.norm()); \
}

DEF_COMPLEX_COMPARE_IM_OPERATOR(<)
DEF_COMPLEX_COMPARE_IM_OPERATOR(<=)
DEF_COMPLEX_COMPARE_IM_OPERATOR(>)
DEF_COMPLEX_COMPARE_IM_OPERATOR(>=)
DEF_COMPLEX_COMPARE_IM_OPERATOR(==)
DEF_COMPLEX_COMPARE_IM_OPERATOR(!=)

#undef DEF_COMPLEX_COMPARE_IM_OPERATOR

template <typename T>
__fast_inline Complex<T> proj(const Complex<T> & m){
    if (std::isinf(m.re) || std::isinf(m.im)) {
        return Complex<T>(std::numeric_limits<T>::infinity(), T(0));
    }else return *m;
}

template <typename T>
__fast_inline Complex<T> conj(const Complex<T> & m){
    return Complex(m.re, -m.im);
}



}

namespace ymd{
    template<typename T>
    __no_inline OutputStream & operator << (OutputStream & os , const UnitComplex<T> & c){
        // return os << c.re << os.splitter() << c.im << 'i';
        return os << c.re << os.splitter() << c.im;
    }

    template<typename T>
    __no_inline OutputStream & operator << (OutputStream & os , const Complex<T> & c){
        // return os << c.re << os.splitter() << c.im << 'i';
        return os << c.re << os.splitter() << c.im;
    }
}

