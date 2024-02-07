#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

class RandomGenerator {
public:
    RandomGenerator();
    void setSeed(unsigned long seed_number);
    float update();

private:
    static const int N = 624;
    static const int M = 397;
    static const unsigned long MATRIX_A = 0x9908b0dfUL;
    static const unsigned long UPPER_MASK = 0x80000000UL;
    static const unsigned long LOWER_MASK = 0x7fffffffUL;

    unsigned long mt[N];
    int mt_index;

    void seed(unsigned long seed_number);
    void generateNumbers();
    unsigned long extractNumber();
};

#endif  // RANDOMGENERATOR_H