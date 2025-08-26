#pragma once

#include "core/platform.hpp"
#include <cstdint>

namespace ymd{


class RandomGenerator {
public:
    RandomGenerator();
    void init(uint32_t seed_number = 0);
    uint32_t update();

private:
    static constexpr int N = 624;
    static constexpr int M = 397;
    static constexpr uint32_t MATRIX_A = 0x9908b0dfUL;
    static constexpr uint32_t UPPER_MASK = 0x80000000UL;
    static constexpr uint32_t LOWER_MASK = 0x7fffffffUL;

    uint32_t mt[N];
    int mt_index;

    void seed(uint32_t seed_number);
    void generateNumbers();
    uint32_t extractNumber();
};

};