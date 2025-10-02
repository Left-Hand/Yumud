#pragma once

#include <cstdint>
#include "core/math/real.hpp"

namespace ymd::details{

template<typename Derived, typename D>
struct _PerUnitCrtp{
    D value;
    constexpr explicit _PerUnitCrtp(const auto _value):value(static_cast<D>(
        D(INVLERP(min(), max(), CLAMP(_value, min(), max())) * std::numeric_limits<D>::max()))){;}

    template<typename T>
    constexpr T to() const {
        return LERP(
            min(), 
            max(), 
            T(value) / std::numeric_limits<D>::max()
        );
    } 

    static constexpr std::weak_ordering compare(const auto other){
        if(unlikely(other > max())) return std::weak_ordering::greater;
        if(unlikely(other < min())) return std::weak_ordering::less;
        return std::weak_ordering::equivalent;
    }

    static constexpr bool is_valid(const D other){
        if(unlikely(other > max())) return false;
        if(unlikely(other < min())) return false;
        return true;
    }
    
private:
    static constexpr D min() {return std::get<0>(Derived::range());}
    static constexpr D max() {return std::get<1>(Derived::range());}
};

}

#define DEF_PER_UNIT(name, dtype, min, max)\
struct name:public ::ymd::details::_PerUnitCrtp<name, dtype>{\
    using _PerUnitCrtp<name, dtype>::_PerUnitCrtp;\
    static constexpr std::tuple<dtype, dtype> range(){\
        return {static_cast<dtype>(min),static_cast<dtype>(max)};}\
};
