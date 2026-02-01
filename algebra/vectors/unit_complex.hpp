#pragma once

#include "primitive/arithmetic/angular.hpp"

#include "core/math/realmath.hpp"
#include "core/stream/ostream.hpp"
#include "algebra/vectors/vec2.hpp"

namespace ymd::math{
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
    UnitComplex from_angle(const Angular<T> angle) {
        const auto [s,c] = angle.sincos();
        return UnitComplex(c, s);
    }

    [[nodiscard]] __fast_inline static constexpr 
    UnitComplex from_radians(const T radians) {
        const auto [s,c] = math::sincos(radians);
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

    [[nodiscard]] constexpr Angular<T> to_angle() const {
        return Angular<T>::from_turns(math::atan2pu(im, re));
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
}
