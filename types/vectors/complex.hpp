#pragma once

#include "core/platform.hpp"
#include "core/utils/angle.hpp"
#include "core/math/realmath.hpp"
#include "types/vectors/Vector2.hpp"

namespace ymd{
template <arithmetic T>
struct UnitComplex {
public:
    T real;
    T imag;


    __fast_inline static constexpr UnitComplex from_uninitialized(){
        return UnitComplex();
    }

    
    __fast_inline constexpr UnitComplex(const T re, const T im) : real(re), imag(im) {;}

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


    [[nodiscard]] constexpr T sine() const {return imag;}
    [[nodiscard]] constexpr T cosine() const {return real;}

    [[nodiscard]] constexpr std::array<T, 2> sincos() const {
        return {sine(), cosine()};
    }

    [[nodiscard]] __fast_inline constexpr T arg() const {return std::atan2(imag, real);}

    __fast_inline constexpr UnitComplex operator-() {
        return UnitComplex(-real, -imag);
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
        return Angle<T>::from_turns(atan2pu(imag, real));
    }

    [[nodiscard]] constexpr Vec2<T> to_vec2(const T length) const {
        return Vec2<T>(real * length, imag * length);
    }

    [[nodiscard]] constexpr Vec2<T> to_unit_vec2() const {
        return Vec2<T>(real, imag);
    }


private:
    constexpr UnitComplex() = default;

    template<size_t I>
    [[nodiscard]] static constexpr auto & get_element(auto & self){
        if constexpr(I == 0){ return self.real; }
        else if constexpr(I == 1){ return self.imag; }
    }
};

template<typename T>
struct Complex{
public:
    T real;
    T imag;


    __fast_inline static constexpr Complex from_uninitialized(){
        return Complex();
    }

    __fast_inline static constexpr Complex from_zero() {return Complex(T(0), T(0));} 
    __fast_inline constexpr Complex(const T & re, const T & im) : real(re), imag(im) {;}


    __fast_inline static constexpr 
    Complex from_imag(const T val) {return Complex(T(0), val);}


    // __fast_inline constexpr UnitComplex<T> normalized(){
    //     const auto ret = *this;
    //     ret /= length(); 
    //     return {ret;
    // }

    [[nodiscard]] __fast_inline constexpr T real_squared() const {return real * real;}
    [[nodiscard]] __fast_inline constexpr T imag_squared() const {return imag * imag;}

    [[nodiscard]] __fast_inline constexpr T length() const {
        return mag(real, imag);
    }

    [[nodiscard]] __fast_inline constexpr T inv_length() const {
        return imag(real, imag);
    }


    [[nodiscard]] constexpr Angle<T> to_angle() const {
        return Angle<T>::from_turns(atan2pu(imag, real));
    }

    template <typename U>
    __fast_inline constexpr Complex& operator=(const Complex<U>& other) {
        if (this != &other) {
            real = static_cast<T>(other.real);
            imag = static_cast<T>(other.imag);
        }
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator+=(const Complex<U>& other) {
        real += static_cast<T>(other.real);
        imag += static_cast<T>(other.imag);
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator+=(const U & other) {
        real += static_cast<T>(other);
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator-=(const Complex<U>& other) {
        real -= static_cast<T>(other.real);
        imag -= static_cast<T>(other.imag);
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator-=(const U & other) {
        real -= static_cast<T>(other);
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator*=(const U & other) {
        real *= other;
        imag *= other;
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator/=(const U & other) {
        real /= other;
        imag /= other;
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator*=(const Complex<U>& other) {
        const auto p_real = real;
        const auto p_imag = imag;
        real = static_cast<T>(static_cast<U>(p_real) * other.real - static_cast<U>(p_imag) * other.imag);
        imag = static_cast<T>(static_cast<U>(p_real) * other.imag + static_cast<U>(p_imag) * other.real);
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator/=(const Complex<U>& other) {
        U denominator = other.norm();
        const auto p_real = real;
        const auto p_imag = imag;
        real = static_cast<T>((static_cast<U>(p_real) * other.real + static_cast<U>(p_imag) * other.imag) / denominator);
        imag = static_cast<T>((static_cast<U>(p_imag) * other.real - static_cast<U>(p_real) * other.imag) / denominator);
        return *this;
    }


private:

    constexpr Complex() = default;

    template<size_t I>
    [[nodiscard]] static constexpr auto & get_element(auto & self){
        if constexpr(I == 0){ return self.real; }
        else if constexpr(I == 1){ return self.imag; }
    }
};
#define COMPLEX_ARITHMETIC_OPERATOR(op)\
\
template <typename T, arithmetic U>\
__fast_inline Complex<T> operator op (Complex<T> lhs, const Complex<U>& rhs) {\
    lhs op##= rhs;\
    return lhs;\
}\
\
template <typename T, arithmetic U>\
__fast_inline Complex<T> operator op (Complex<T> lhs, const U & rhs) {\
    lhs op##= static_cast<T>(rhs);\
    return lhs;\
}\
\
template <typename T, arithmetic U>\
__fast_inline Complex<T> operator op (const U & lhs, Complex<T> rhs) {\
    Complex<T>(static_cast<T>(lhs), T(0)) op##= rhs;\
    return lhs;\
}

COMPLEX_ARITHMETIC_OPERATOR(+)
COMPLEX_ARITHMETIC_OPERATOR(-)
COMPLEX_ARITHMETIC_OPERATOR(*)
COMPLEX_ARITHMETIC_OPERATOR(/)


template <typename T, arithmetic U>
__fast_inline bool operator==(const Complex<T>& lhs, const Complex<U>& rhs) {
    return lhs.real == static_cast<T>(rhs.real) && lhs.imag == static_cast<T>(rhs.imag);
}

template <typename T>
__fast_inline bool operator!=(const Complex<T>& lhs, const Complex<T>& rhs) {
    return !(lhs == rhs);
}

#define DEF_COMPLEX_COMPARE_IM_OPERATOR(op) \
\
template <typename T, arithmetic U> \
__fast_inline bool operator op (const Complex<T>& lhs, const U& rhs) { \
    T absrhs = static_cast<T>(length(rhs)); \
    return lhs.norm() op (absrhs * absrhs); \
} \
\
template <typename T, arithmetic U> \
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
    if (std::isinf(m.real) || std::isinf(m.imag)) {
        return Complex<T>(std::numeric_limits<T>::infinity(), T(0));
    }else return *m;
}

template <typename T>
__fast_inline Complex<T> conj(const Complex<T> & m){
    return Complex(m.real, -m.imag);
}



}

namespace ymd{
    template<typename T>
    __no_inline OutputStream & operator << (OutputStream & os , const UnitComplex<T> & c){
        // return os << c.real << os.splitter() << c.imag << 'i';
        return os << c.real << os.splitter() << c.imag;
    }

    template<typename T>
    __no_inline OutputStream & operator << (OutputStream & os , const Complex<T> & c){
        // return os << c.real << os.splitter() << c.imag << 'i';
        return os << c.real << os.splitter() << c.imag;
    }
}

