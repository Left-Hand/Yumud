#pragma once

#include "core/math/real.hpp"

namespace ymd::curve{

template<typename T>
concept Functor = requires(T f, iq16 x) {
    { f(x) } -> std::same_as<iq16>;
};

template<typename T>
class CurveIntf{
protected:
    const T _from;
    const T _delta;
public:
    using Type = T;
    CurveIntf(const CurveIntf & other) = default;
    CurveIntf(CurveIntf && other) = default;

    CurveIntf(const T & from,const T & to):
        _from(from), _delta(to - from) {}

    virtual ~CurveIntf() = default;

    virtual T forward(const iq16 t) const = 0;

    virtual iq16 period() const = 0;

    virtual T operator()(const iq16 t) const {
        return forward(t);
    }

};

// template<typename T>
// concept is_curve = std::is_base_of_v<CurveIntf<T::Type>, T>;

template<typename T, Functor U>
class CurveFunctor_t:public CurveIntf<T>{
    const iq16 _dur;
    const U _functor; 
public:
    CurveFunctor_t(const T & from,const T & to, const iq16 dur, auto && functor) :
        CurveIntf<T>(from, to),
        _dur(dur),
        _functor(functor)
        {}
    T forward(const iq16 x) const override{
        return this->_from + this->_delta * iq16(_functor(x / _dur));
    }

    iq16 period() const override{ return _dur; }
};

// template<typename T, Functor U>
// auto make_curve(const T & from, const T & to, const iq16 dur, U && functor) {
//     using Func = std::remove_reference_t<decltype(functor)>;
//     return CurveFunctor_t<std::conditional_t<std::is_arithmetic_v<T>, iq16, T>, Func>(from, to, dur, std::forward<Func>(functor));
// }

// template<template<typename> class CurveType, typename T>
// auto make_curve(const T & from, const T & to, auto && ... args) {
//     return CurveType<std::conditional_t<std::is_arithmetic_v<T>, iq16, T>>{from, to, std::forward<decltype(args)>(args)...};
// }

}