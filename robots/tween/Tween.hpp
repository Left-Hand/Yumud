#pragma once

#include "sys/utils/setget/Setter.hpp"
#include "robots/curve/CurveConcept_t.hpp"


namespace ymd::tween{

// template<typename Setter, typename Curve>
// class Tweener_t{
//     Setter _setter;
//     Curve _curve; 
// public:
//     Tweener_t(Setter && setter, Curve && curve):
//         _setter(std::move(setter)),
//         _curve(std::move(curve)){}

//     Tweener_t(const Setter & setter,const Curve & curve):
//         _setter(setter),
//         _curve(curve){}

//     void update(const real_t time){
//         // auto && res = _curve(time);
//         _setter( _curve(time));
//     }

//     real_t period() const{
//         return _curve.period();
//     }
// };

// namespace test{
template<typename T>
class Tweener_t{

    using Setter = ymd::utils::SetterConcept_t<T>;
    using Curve = ymd::curve::CurveConcept_t<T>;

    using SetterWrapper = std::unique_ptr<Setter>;
    using CurveWrapper = std::unique_ptr<Curve>;

    SetterWrapper _setter;
    CurveWrapper _curve; 
public:
    // Tweener_t(const auto & setter, const auto & curve):
    //     _setter(new Setter(std::forward<decltype(setter)>(setter))),
    //     _curve(new Curve(std::forward<decltype(curve)>(curve))){}
    // Tweener_t(auto && setter, auto && curve):
    //     _setter(std::make_unique<std::remove_reference_t<decltype(setter)>>(setter)),
    //     _curve(std::make_unique<std::remove_reference_t<decltype(curve)>>(curve)){}

    template<typename SetterType, typename CurveType>
    Tweener_t(const SetterType & setter, const CurveType & curve)
        : _setter(std::make_unique<SetterType>((setter))),
          _curve(std::make_unique<CurveType>((curve))) {}


    // Tweener_t(auto && setter, auto && curve):
    //     _setter(nullptr),
    //     _curve(nullptr){}

    void update(const real_t time){
        (*_setter)( (*_curve)(time));
    }

    real_t period() const{
        return _curve->period();
    }
};
// }

// template<typename T>
// auto make_tweener(ymd::utils::SetterConcept_t<T> && setter, ymd::utils::CurveConcept_t<T> && curve){
//     return Tweener_t<T>(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
// }

template<typename T>
auto make_tweener(auto && setter, auto && curve){
    return Tweener_t<T>(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
}

template<typename T>
auto new_tweener(auto && setter, auto && curve){
    return new Tweener_t<T>(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
}

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


}