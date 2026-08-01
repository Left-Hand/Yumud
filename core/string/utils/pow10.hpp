#pragma once

#include <cstdint>
#include <type_traits>
#include <array>

namespace ymd::str{

// \frac{32\ln\left(2\right)}{\ln\left(10\right)} <= 10
static constexpr std::array<uint32_t, 10> POW10_TABLE = {
    1UL, 
    10UL, 
    100UL, 
    1000UL, 

    10000UL, 
    100000UL, 
    1000000UL, 
    10000000UL, 
    100000000UL,
    1000000000UL
};



}