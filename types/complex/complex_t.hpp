#pragma once

#include "../sys/core/platform.h"
#include <type_traits>

template <typename T>
requires std::is_arithmetic_v<T>
struct Complex_t {
public:
    T real;
    T imag;

    __fast_inline T real_squared() const {return real * real;}
    __fast_inline T imag_squared() const {return imag * imag;}
    __fast_inline T norm() const {return real * real + imag * imag;}
    __fast_inline T abs() const {return sqrt(real * real + imag * imag);}
    __fast_inline T arg() const {return std::atan2(imag, real);}
    __fast_inline Complex_t & normalized(){*this /= abs(); return *this;}

    __fast_inline explicit Complex_t() : real(T()), imag(T()) {;}
    __fast_inline explicit Complex_t(const T & im) : real(T()), imag(im) {;}
    __fast_inline explicit Complex_t(const T & re, const T & im) : real(re), imag(im) {;}

    template <typename U>
	__fast_inline Complex_t(const U & im) {
		imag = static_cast<T>(im);
	}

    template <typename U>
	__fast_inline Complex_t(const U & re,const U & im) {
		real = static_cast<T>(re);
		imag = static_cast<T>(im);
	}

    __fast_inline Complex_t operator-() {
        return Complex_t(-real, -imag);
    }

    template <typename U>
    __fast_inline Complex_t& operator=(const Complex_t<U>& other) {
        if (this != &other) {
            real = static_cast<T>(other.real);
            imag = static_cast<T>(other.imag);
        }
        return *this;
    }

    template <typename U>
    __fast_inline Complex_t& operator+=(const Complex_t<U>& other) {
        real += static_cast<T>(other.real);
        imag += static_cast<T>(other.imag);
        return *this;
    }

    template <typename U>
    __fast_inline Complex_t& operator+=(const U & other) {
        real += static_cast<T>(other);
        return *this;
    }

    template <typename U>
    __fast_inline Complex_t& operator-=(const Complex_t<U>& other) {
        real -= static_cast<T>(other.real);
        imag -= static_cast<T>(other.imag);
        return *this;
    }

    template <typename U>
    __fast_inline Complex_t& operator-=(const U & other) {
        real -= static_cast<T>(other);
        return *this;
    }

    template <typename U>
    __fast_inline Complex_t& operator*=(const U & other) {
        real *= other;
        imag *= other;
        return *this;
    }

    template <typename U>
    __fast_inline Complex_t& operator/=(const U & other) {
        real /= other;
        imag /= other;
        return *this;
    }

    template <typename U>
    __fast_inline Complex_t& operator*=(const Complex_t<U>& other) {
        T orgReal = real;
        T orgImag = imag;
        real = static_cast<T>(static_cast<U>(orgReal) * other.real - static_cast<U>(orgImag) * other.imag);
        imag = static_cast<T>(static_cast<U>(orgReal) * other.imag + static_cast<U>(orgImag) * other.real);
        return *this;
    }

    template <typename U>
    __fast_inline Complex_t& operator/=(const Complex_t<U>& other) {
        U denominator = other.norm();
        T orgReal = real;
        T orgImag = imag;
        real = static_cast<T>((static_cast<U>(orgReal) * other.real + static_cast<U>(orgImag) * other.imag) / denominator);
        imag = static_cast<T>((static_cast<U>(orgImag) * other.real - static_cast<U>(orgReal) * other.imag) / denominator);
        return *this;
    }

    __no_inline explicit operator String() const{
        if(imag > 0) return (String(static_cast<float>(real)) + String('+') + String(static_cast<float>(imag)) + String('i'));
        else if(imag < 0)return (String(static_cast<float>(real)) + String(static_cast<float>(-imag)) + String('i'));
        else return String(static_cast<float>(real));
    }

    __no_inline String toString(unsigned char decimalPlaces = 2){
        if(imag > 0) return (String(static_cast<float>(imag), decimalPlaces) + String('+') + String(static_cast<float>(imag), decimalPlaces) + String('i'));
        else if(imag < 0)return (String(static_cast<float>(imag), decimalPlaces) + String(static_cast<float>(-imag), decimalPlaces) + String('i'));
        else return String(static_cast<float>(imag), decimalPlaces);
    }
};

#include "complex_t.tpp"
