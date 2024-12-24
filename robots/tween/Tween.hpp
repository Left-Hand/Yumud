#pragma once

#include "sys/utils/setget/Setter.hpp"
#include "robots/curve/CurveConcept_t.hpp"


namespace ymd::tween{

namespace internal{
    PRO_DEF_MEM_DISPATCH(MemPeriod, period);
    PRO_DEF_MEM_DISPATCH(MemUpdate, update);

    // template<typename T>
    struct TweenerFacade : pro::facade_builder
        ::add_convention<internal::MemPeriod, real_t() const>
        ::add_convention<internal::MemUpdate, void(real_t)>
        ::build {};
}
// template<typename T>
using TweenerProxy = pro::proxy<internal::TweenerFacade>;

template<typename T>
class TweenerConcept_t{
public:
    using Setter = ymd::utils::SetterConcept_t<T>;
    using Curve = ymd::curve::CurveConcept_t<T>;

    virtual void update(const real_t time) = 0;
    virtual real_t period() const = 0;

    TweenerProxy operator &(){
        return TweenerProxy(this);
    }
};


template<typename T>
// class TweenerStatic_t{
class TweenerStatic_t:public TweenerConcept_t<T>{
public:
    using Setter = TweenerConcept_t<T>::Setter;
    using Curve = TweenerConcept_t<T>::Curve;

    Setter & _setter;
    Curve & _curve; 
public:
    TweenerStatic_t(Setter & setter, Curve & curve):
        _setter(setter),
        _curve(curve){}

    void update(const real_t time) {
        // auto && res = _curve(time);
        _setter( _curve(time));
    }

    real_t period() const {
        return _curve.period();
    }
};



template<typename T>
class Tweener_t:public TweenerConcept_t<T>{
public:
    using Setter = TweenerConcept_t<T>::Setter;
    using Curve = TweenerConcept_t<T>::Curve;

    using SetterWrapper = std::shared_ptr<Setter>;
    using CurveWrapper = std::shared_ptr<Curve>;
protected:
    SetterWrapper _setter;
    CurveWrapper _curve; 
public:

    template<typename SetterType, typename CurveType>
    Tweener_t(const SetterType & setter, const CurveType & curve)
        : _setter(std::make_shared<SetterType>((setter))),
          _curve(std::make_shared<CurveType>((curve))) {}

    Tweener_t(const SetterWrapper setter, const CurveWrapper curve)
        : _setter(setter),
          _curve(curve){}

    void update(const real_t time) override {
        if(_setter == nullptr or _curve == nullptr) HALT;
        (*_setter)( (*_curve)(time));
    }

    real_t period() const override{
        return _curve->period();
    }
};



template<typename T>
auto make_tweener(auto && setter, auto && curve){
    return Tweener_t<T>(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
}

template<typename T>
auto make_twproxy(auto && setter, auto && curve){
    return pro::make_proxy<internal::TweenerFacade, Tweener_t<T>>(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
}

// template<typename T>
// auto new_tweener(auto && setter, auto && curve){
//     return new Tweener_t<T>(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
// }

// auto new_tweener(auto && setter, auto && curve){
//     return new Tweener_t(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
// }


template<typename ValueType, typename Interpolator>
auto make_tweener(
    auto && obj, 
    void (std::remove_reference_t<decltype(obj)>::*member_func_ptr)(const ValueType &),
    const real_t dur,
    const std::conditional_t<std::is_arithmetic_v<ValueType>, real_t, ValueType> & from, 
    const std::conditional_t<std::is_arithmetic_v<ValueType>, real_t, ValueType> & to,
    const Interpolator & interpolator 
    )
{
    auto setter = ymd::utils::make_setter(obj, member_func_ptr);
    auto curve = ymd::curve::make_curve(from, to, dur, interpolator);

    return Tweener_t<std::conditional_t<std::is_arithmetic_v<ValueType>, real_t, ValueType>>(
        std::move(setter), std::move(curve));
}

template<typename ValueType, typename Interpolator>
auto make_twproxy(
    auto && obj, 
    void (std::remove_reference_t<decltype(obj)>::*member_func_ptr)(const ValueType &),
    const real_t dur,
    const std::conditional_t<std::is_arithmetic_v<ValueType>, real_t, ValueType> & from, 
    const std::conditional_t<std::is_arithmetic_v<ValueType>, real_t, ValueType> & to,
    const Interpolator & interpolator 
    )
{
    auto setter = ymd::utils::make_setter(obj, member_func_ptr);
    auto curve = ymd::curve::make_curve(from, to, dur, interpolator);

    return pro::make_proxy<internal::TweenerFacade, Tweener_t<std::conditional_t<std::is_arithmetic_v<ValueType>, real_t, ValueType>>>(
        std::move(setter), std::move(curve));
}



}