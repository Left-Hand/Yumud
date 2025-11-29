#pragma once

#include "core/utils/setget/Setter.hpp"
#include "robots/curve/CurveConcept_t.hpp"

#if 0

namespace ymd::tween{

namespace details{
    PRO_DEF_MEM_DISPATCH(MemPeriod, period);
    PRO_DEF_MEM_DISPATCH(MemUpdate, update);

    // template<typename T>
    struct TweenerFacade : pro::facade_builder
        ::add_convention<details::MemPeriod, real_t() const>
        ::add_convention<details::MemUpdate, void(real_t)>
        ::build {};
}
// template<typename T>
using TweenerProxy = pro::proxy<details::TweenerFacade>;

template<typename T>
class TweenerIntf{
public:
    using Setter = ymd::utils::SetterIntf_t<T>;
    using Curve = ymd::curve::CurveIntf<T>;

    virtual void update(const real_t time) = 0;
    virtual real_t period() const = 0;

    TweenerProxy operator &(){
        return TweenerProxy(this);
    }
};


template<typename T>
// class TweenerStatic{
class TweenerStatic:public TweenerIntf<T>{
public:
    using Setter = TweenerIntf<T>::Setter;
    using Curve = TweenerIntf<T>::Curve;

    Setter & setter_;
    Curve & curve_; 
public:
    TweenerStatic(Setter & setter, Curve & curve):
        setter_(setter),
        curve_(curve){}

    void update(const real_t time) {
        // auto && res = curve_(time);
        setter_( curve_(time));
    }

    real_t period() const {
        return curve_.period();
    }
};



template<typename T>
class Tweener:public TweenerIntf<T>{
public:
    using Setter = TweenerIntf<T>::Setter;
    using Curve = TweenerIntf<T>::Curve;

    using SetterWrapper = std::unique_ptr<Setter>;
    using CurveWrapper = std::unique_ptr<Curve>;
protected:
    SetterWrapper setter_;
    CurveWrapper curve_; 
public:

    template<typename SetterType, typename CurveType>
    Tweener(const SetterType & setter, const CurveType & curve): 
        setter_(std::make_unique<SetterType>((setter))),
        curve_(std::make_unique<CurveType>((curve))) {}

    Tweener(const SetterWrapper setter, const CurveWrapper curve): 
        setter_(setter),
        curve_(curve){}

    void update(const real_t time) override {
        if(setter_ == nullptr or curve_ == nullptr) HALT;
        (*setter_)( (*curve_)(time));
    }

    real_t period() const override{
        return curve_->period();
    }
};



template<typename T>
auto make_tweener(auto && setter, auto && curve){
    return Tweener<T>(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
}

template<typename T>
auto make_twproxy(auto && setter, auto && curve){
    return pro::make_proxy<details::TweenerFacade, Tweener<T>>(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
}

// template<typename T>
// auto new_tweener(auto && setter, auto && curve){
//     return new Tweener<T>(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
// }

// auto new_tweener(auto && setter, auto && curve){
//     return new Tweener(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
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

    return Tweener<std::conditional_t<std::is_arithmetic_v<ValueType>, real_t, ValueType>>(
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

    return pro::make_proxy<details::TweenerFacade, Tweener<std::conditional_t<std::is_arithmetic_v<ValueType>, real_t, ValueType>>>(
        std::move(setter), std::move(curve));
}
}

#endif