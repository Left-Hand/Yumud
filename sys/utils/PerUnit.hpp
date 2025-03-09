#pragma once

#include <cstdint>
#include "sys/math/real.hpp"

namespace ymd::details{

template<typename T, typename D>
struct _PerUnit{
    D data;
    constexpr _PerUnit(const real_t value):data(static_cast<D>(
        real_t(INVLERP(get_min(), get_max(), CLAMP(value, get_min(), get_max())) * std::numeric_limits<D>::max()))){;}

    constexpr operator real_t() const {return LERP(get_min(), get_max(), real_t(data) / std::numeric_limits<D>::max());} 

    static constexpr int check(const real_t value){
        if(unlikely(value > get_max())) return 1;
        if(unlikely(value < get_min())) return -1;
        return 0;
    }

    // operator <=>operator ymd::iq_t<16U>
private:
    static constexpr real_t get_min() {return std::get<0>(T::get_range());}
    static constexpr real_t get_max() {return std::get<1>(T::get_range());}
};

}

#define DEF_PER_UNIT(name, dtype, min, max)\
struct name:public ::ymd::details::_PerUnit<name, dtype>{\
    static constexpr std::tuple<real_t, real_t> get_range(){return {static_cast<real_t>(min),static_cast<real_t>(max)};}\
};
