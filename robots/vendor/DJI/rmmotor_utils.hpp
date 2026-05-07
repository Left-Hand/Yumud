#pragma once

#include <bitset>
#include <type_traits>

#include "core/math/fixed/fixed.hpp"
#include "primitive/arithmetic/angular.hpp"


namespace ymd::robots::dji{


namespace utils{


template<typename T>
struct scale_1_by_60;

template<typename T>
requires(std::is_floating_point_v<T>)
struct scale_1_by_60<T>{
    [[nodiscard]] static constexpr T calc(int16_t x){
    constexpr float FACTOR = 1.0f / 60;
    return (x * FACTOR);
    }
};

template<size_t Q>
struct scale_1_by_60<math::fixed<Q, int32_t>>{
    using T = math::fixed<Q, int32_t>;

    [[nodiscard]] static constexpr T calc(int16_t x){
        const auto temp = math::fixed<Q, int32_t>(x) * uq32(1.0 / 60);
        return temp;
    }
};



}


};