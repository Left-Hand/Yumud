#include "random_generator.hpp"

using namespace yumud;

RandomGenerator::RandomGenerator(){
    seed(0);
}

void RandomGenerator::init(uint32_t seed_number){
    seed(seed_number);
}

uint32_t RandomGenerator::update(){
    generateNumbers();
    return extractNumber();
}

void RandomGenerator::seed(uint32_t seed_number)
{
    mt[0] = seed_number & 0xffffffffUL;
    for (mt_index = 1; mt_index < N; mt_index++) {
        mt[mt_index] = (1812433253UL * (mt[mt_index - 1] ^ (mt[mt_index - 1] >> 30)) +
                        mt_index);
        mt[mt_index] &= 0xffffffffUL;
    }
}

void RandomGenerator::generateNumbers()
{
    for (int i = 0; i < N; i++) {
        uint32_t y = (mt[i] & UPPER_MASK) | (mt[(i + 1) % N] & LOWER_MASK);
        mt[i] = mt[(i + M) % N] ^ (y >> 1);
        if (y % 2 != 0) {
            mt[i] ^= MATRIX_A;
        }
    }
}

uint32_t RandomGenerator::extractNumber()
{
    if (mt_index == 0) {
        generateNumbers();
    }

    uint32_t y = mt[mt_index];
    y ^= (y >> 11);
    y ^= ((y << 7) & 0x9d2c5680UL);
    y ^= ((y << 15) & 0xefc60000UL);
    y ^= (y >> 18);

    mt_index = (mt_index + 1) % N;

    return y;
}