#include "random_generator.hpp"

RandomGenerator::RandomGenerator()
{
    seed(0);
}

void RandomGenerator::setSeed(unsigned long seed_number)
{
    seed(seed_number);
}

float RandomGenerator::update()
{
    generateNumbers();
    unsigned long value = extractNumber();
    return static_cast<float>(value) / 4294967295.0f;
}

void RandomGenerator::seed(unsigned long seed_number)
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
        unsigned long y = (mt[i] & UPPER_MASK) | (mt[(i + 1) % N] & LOWER_MASK);
        mt[i] = mt[(i + M) % N] ^ (y >> 1);
        if (y % 2 != 0) {
            mt[i] ^= MATRIX_A;
        }
    }
}

unsigned long RandomGenerator::extractNumber()
{
    if (mt_index == 0) {
        generateNumbers();
    }

    unsigned long y = mt[mt_index];
    y ^= (y >> 11);
    y ^= ((y << 7) & 0x9d2c5680UL);
    y ^= ((y << 15) & 0xefc60000UL);
    y ^= (y >> 18);

    mt_index = (mt_index + 1) % N;

    return y;
}