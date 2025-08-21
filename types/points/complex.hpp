#pragma once

#include "core/platform.hpp"

namespace ymd{
template <arithmetic T>
struct Complex {
public:
    T real;
    T imag;


    __fast_inline constexpr Complex & normalized(){*this /= abs(); return *this;}

    __fast_inline constexpr Complex() : real(T(0)), imag(T(0)) {;}
    __fast_inline constexpr Complex(const T & re, const T & im) : real(re), imag(im) {;}


    
    __fast_inline static constexpr 
    Complex i() {return Complex(T(0), T(1));}

    __fast_inline static constexpr 
    Complex from_imag(const T val) {return Complex(T(0), val);}

    __fast_inline static constexpr 
    Complex from_angle(const T val) {
        const auto [s,c] = sincos(val);
        return Complex(c, s);
    }

    [[nodiscard]] __fast_inline constexpr T real_squared() const {return real * real;}
    [[nodiscard]] __fast_inline constexpr T imag_squared() const {return imag * imag;}
    [[nodiscard]] __fast_inline constexpr T norm() const {return real * real + imag * imag;}
    [[nodiscard]] __fast_inline constexpr T arg() const {return std::atan2(imag, real);}

    __fast_inline constexpr Complex operator-() {
        return Complex(-real, -imag);
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
        T orgReal = real;
        T orgImag = imag;
        real = static_cast<T>(static_cast<U>(orgReal) * other.real - static_cast<U>(orgImag) * other.imag);
        imag = static_cast<T>(static_cast<U>(orgReal) * other.imag + static_cast<U>(orgImag) * other.real);
        return *this;
    }

    template <typename U>
    __fast_inline constexpr Complex& operator/=(const Complex<U>& other) {
        U denominator = other.norm();
        T orgReal = real;
        T orgImag = imag;
        real = static_cast<T>((static_cast<U>(orgReal) * other.real + static_cast<U>(orgImag) * other.imag) / denominator);
        imag = static_cast<T>((static_cast<U>(orgImag) * other.real - static_cast<U>(orgReal) * other.imag) / denominator);
        return *this;
    }

    __fast_inline constexpr T abs() const {
        return mag(real, imag);
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
    return lhs.real == static_cast<T>(rhs.real) && lhs.imag == static_cast<T>(rhs.imag);
}

template <typename T>
__fast_inline bool operator!=(const Complex<T>& lhs, const Complex<T>& rhs) {
    return !(lhs == rhs);
}

#define DEF_COMPLEX_COMPARE_IM_OPERATOR(op) \
\
template <typename T, typename U> \
__fast_inline bool operator op (const Complex<T>& lhs, const U& rhs) { \
    T absrhs = static_cast<T>(abs(rhs)); \
    return lhs.norm() op (absrhs * absrhs); \
} \
\
template <typename T, typename U> \
__fast_inline bool operator op (const U& lhs, const Complex<T>& rhs) { \
    T abslhs = static_cast<T>(abs(lhs)); \
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
    __no_inline OutputStream & operator << (OutputStream & os , const Complex<T> & c){
        // return os << c.real << os.splitter() << c.imag << 'i';
        return os << c.real << os.splitter() << c.imag;
    }
}

