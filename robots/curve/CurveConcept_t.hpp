#pragma once

#include "sys/math/real.hpp"

namespace ymd::curve{

template<typename T>
concept Functor = requires(T f, real_t x) {
    { f(x) } -> std::same_as<real_t>;
};

template<typename T>
class CurveConcept_t{
protected:
    const T _from;
    const T _delta;
public:
    using Type = T;
    CurveConcept_t(const CurveConcept_t & other) = default;
    CurveConcept_t(CurveConcept_t && other) = default;

    CurveConcept_t(const T & from,const T & to):
        _from(from), _delta(to - from) {}

    virtual ~CurveConcept_t() = default;

    virtual T forward(const real_t t) const = 0;

    virtual real_t period() const = 0;

    virtual T operator()(const real_t t) const {
        return forward(t);
    }

};

// template<typename T>
// concept is_curve = std::is_base_of_v<CurveConcept_t<T::Type>, T>;

template<typename T, Functor U>
class CurveFunctor_t:public CurveConcept_t<T>{
    const real_t _dur;
    const U _functor; 
public:
    CurveFunctor_t(const T & from,const T & to, const real_t dur, auto && functor) :
        CurveConcept_t<T>(from, to),
        _dur(dur),
        _functor(functor)
        {}
    T forward(const real_t x) const override{
        return this->_from + this->_delta * real_t(_functor(x / _dur));
    }

    real_t period() const override{ return _dur; }
};

template<typename T, Functor U>
auto make_curve(const T & from, const T & to, const real_t dur, U && functor) {
    using Func = std::remove_reference_t<decltype(functor)>;
    return CurveFunctor_t<std::conditional_t<std::is_arithmetic_v<T>, real_t, T>, Func>(from, to, dur, std::forward<Func>(functor));
}

template<template<typename> class CurveType, typename T>
auto make_curve(const T & from, const T & to, auto && ... args) {
    return CurveType<std::conditional_t<std::is_arithmetic_v<T>, real_t, T>>{from, to, std::forward<decltype(args)>(args)...};
}

}