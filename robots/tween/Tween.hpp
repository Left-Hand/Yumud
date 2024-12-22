#pragma once

#include "sys/utils/setget/Setter.hpp"


namespace ymd::tween{

template<typename Setter, typename Curve>
class Tweener_t{
    Setter _setter;
    Curve _curve; 
public:
    Tweener_t(Setter && setter, Curve && curve):
        _setter(std::move(setter)),
        _curve(std::move(curve)){}

    Tweener_t(const Setter & setter,const Curve & curve):
        _setter(setter),
        _curve(curve){}

    void update(const real_t time){
        // auto && res = _curve(time);
        _setter( _curve(time));
    }

    real_t period() const{
        return _curve.period();
    }
};


auto make_tweener(auto && setter, auto && curve){
    return Tweener_t(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
}

auto new_tweener(auto && setter, auto && curve){
    return new Tweener_t(std::forward<decltype(setter)>(setter), std::forward<decltype(curve)>(curve));
}


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

    return Tweener_t<decltype(setter), decltype(curve)>(std::move(setter), std::move(curve));
}


}