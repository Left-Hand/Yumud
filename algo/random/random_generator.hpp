#pragma once

#include "core/platform.h"
#include <cstdint>

namespace ymd{


class RandomGenerator {
public:
    RandomGenerator();
    void init(uint32_t seed_number = 0);
    uint32_t update();

private:
    scexpr int N = 624;
    scexpr int M = 397;
    scexpr uint32_t MATRIX_A = 0x9908b0dfUL;
    scexpr uint32_t UPPER_MASK = 0x80000000UL;
    scexpr uint32_t LOWER_MASK = 0x7fffffffUL;

    uint32_t mt[N];
    int mt_index;

    void seed(uint32_t seed_number);
    void generateNumbers();
    uint32_t extractNumber();
};

};