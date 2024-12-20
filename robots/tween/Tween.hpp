#pragma once

#include "sys/utils/setget/Setter.hpp"


namespace ymd::tween{

template<typename Setter, typename Curve>
// requires std::is_same_v<Setter::Type, Curve::Type>
// requires std::is_same_v(Setter::Type, Curve::Type)
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
};


template<typename T, typename U>
auto make_tweener(const T & setter, const U & curve){
    return Tweener_t<T, U>(setter, curve);
}

template<typename ValueType, typename Interpolator>
auto make_tweener(
    auto && obj, 
    void (std::remove_reference_t<decltype(obj)>::*member_func_ptr)(const ValueType &),
    const Interpolator & interpolator, 
    const std::conditional_t<std::is_arithmetic_v<ValueType>, real_t, ValueType> & from, 
    const std::conditional_t<std::is_arithmetic_v<ValueType>, real_t, ValueType> & to)
{
    auto setter = ymd::utils::make_setter(obj, member_func_ptr);
    auto curve = ymd::curve::make_curve(interpolator, from, to);

    return Tweener_t<decltype(setter), decltype(curve)>(std::move(setter), std::move(curve));
}


}