#pragma once

#include "interpolation.hpp"

namespace ymd::intp{

class CosineInterpolation:public Interpolation{
public:
    real_t forward(real_t x) const override;
};


class SeatInterpolation:public Interpolation{
protected:
    static std::tuple<real_t, real_t> get_ab(const Vec2<q16> & handle);
protected:
    real_t a;
    real_t b;
public:
    SeatInterpolation(const Vec2<q16> & handle){
        std::tie(a,b) = get_ab(handle);
    }

    real_t forward(const real_t x) const override;
};

class SeatLineInterpolation:public SeatInterpolation{
public:
    using SeatInterpolation::SeatInterpolation;
    real_t forward(const real_t x) const override;
};

class SeatOddInterpolation:public SeatInterpolation{
protected:
    int p;
public:
    SeatOddInterpolation(const Vec2<q16> & handle, const int n):
        SeatInterpolation(handle),p(2 * n + 1){}

    real_t forward(const real_t x) const override;
};


class SymmetricInterpolation:public SeatInterpolation{
protected:
    int _n;
public:
    SymmetricInterpolation(const Vec2<q16> & handle, const int n):
        SeatInterpolation(handle),_n(n){}
    real_t forward(const real_t x) const override;
};

class QuadraticSeatInterpolation:public SeatInterpolation{
protected:
    int _n;
public:
    QuadraticSeatInterpolation(const Vec2<q16> & handle, const int n):
        SeatInterpolation(handle),_n(n){}
    real_t forward(const real_t x) const override;
};



}