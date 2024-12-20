#pragma once

#include "sys/math/real.hpp"

namespace ymd::curve{

template<typename T>
concept Functor = requires(T f, real_t x) {
    { f(x) } -> std::same_as<real_t>;
};

template<typename T>
class CurveConcept_t{
public:
    using Type = T;
    CurveConcept_t(const CurveConcept_t & other) = default;
    CurveConcept_t(CurveConcept_t && other) = default;

    CurveConcept_t() = default;
    virtual ~CurveConcept_t() = default;

    virtual T operator()(const real_t x) const = 0;
};

template<typename T, Functor U>
class CurveFunctor_t:public CurveConcept_t<T>{
    const U _functor; 
    const T from_;
    const T delta_;
    const real_t dur_;
public:
    CurveFunctor_t(U && functor, const T & from,const T & to, const real_t dur = 1) :
        _functor(std::move(functor)),
        from_(from), 
        delta_(to - from),
        dur_(dur)
        {}

    CurveFunctor_t(const U & functor, const T & from,const T & to, const real_t dur = 1) :
        _functor((functor)),
        from_(from), 
        delta_(to - from),
        dur_(dur)
        {}

    T operator()(const real_t x) const override{
        return from_ + delta_ * real_t(_functor(x));
    }
};

template<arithmetic T>
auto make_curve(Functor auto && functor, const T & from, const T & to) {
    using Func = std::remove_reference_t<decltype(functor)>;
    return CurveFunctor_t<real_t, Func>(std::forward<Func>(functor), from, to);
}

template<typename T>
auto make_curve(Functor auto && functor, const T & from, const T & to) {
    using Func = std::remove_reference_t<decltype(functor)>;
    return CurveFunctor_t<T, Func>(std::forward<Func>(functor), from, to);
}

}