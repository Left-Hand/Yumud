#pragma once

#include "../fxmath/fconv.hpp"
#include "../fxmath/div.hpp"
#include "../fixed.hpp"


#include <cmath>

using namespace ymd;
using namespace ymd::fxmath;
using namespace ymd::fxmath::details;
using namespace ymd::literals;



template<std::floating_point T>
static constexpr T abs_err(const T a, const T b){
    if(a > b) return a - b;
    return b - a;
}


static constexpr int64_t err64(const uint64_t a, const uint64_t b){
    if(a > b) return a - b;
    else return b - a;
}


template<size_t Q, typename T>
static consteval double err_f64(const math::fixed<Q, T> a, const math::fixed<Q, T> b){
    // if(a > b) return a - b;
    // else return b - a;
    if(b > a) return (double)(b.to_bits() - a.to_bits()) / static_cast<double>(1ull << Q);
    return (double)(a.to_bits() - b.to_bits()) / static_cast<double>(1ull << Q);
}
