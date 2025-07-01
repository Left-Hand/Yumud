#pragma once

#include "core/platform.hpp"

namespace ymd{
template <arithmetic T>
struct Complex {
public:
    T real;
    T imag;

    __fast_inline constexpr T real_squared() const {return real * real;}
    __fast_inline constexpr T imag_squared() const {return imag * imag;}
    __fast_inline constexpr T norm() const {return real * real + imag * imag;}
    __fast_inline constexpr T abs() const {return sqrt(real * real + imag * imag);}
    __fast_inline constexpr T arg() const {return std::atan2(imag, real);}
    __fast_inline constexpr Complex & normalized(){*this /= abs(); return *this;}

    __fast_inline constexpr Complex() : real(T(0)), imag(T(0)) {;}
    __fast_inline constexpr Complex(const T & re, const T & im) : real(re), imag(im) {;}

    __fast_inline static constexpr 
    Complex i() {return Complex(T(0), T(1));}

    __fast_inline static constexpr 
    Complex from_imag(const T val) {return Complex(T(0), val);}

    __fast_inline static constexpr 
    Complex from_polar(const T val) {
        const auto [s,c] = sincos(val);
        return Complex(c, s);
    }

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
};



#include "Complex.tpp"

}

namespace ymd{
    __no_inline OutputStream & operator << (OutputStream & os , const Complex<auto> & c){
        return os << c.real << os.splitter() << c.imag << 'i';
    }
}

