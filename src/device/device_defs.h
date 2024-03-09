#ifndef __DEVICE_INC_H__
#define __DEVICE_INC_H__

#include "platform.h"
#include "src/debug/debug_inc.h"

#ifndef REG8_BEGIN
#define REG8_BEGIN union{struct{
#endif

#ifndef REG8_END
#define REG8_END };uint8_t data;};
#endif

#ifndef REG16_BEGIN
#define REG16_BEGIN union{struct{
#endif

#ifndef REG16_END
#define REG16_END };uint16_t data;};
#endif

struct Reg8{};
struct Reg16{};

struct Fraction {
public:
    int numerator;
    int denominator;
    Fraction(int numerator, int denominator) : numerator(numerator), denominator(denominator) {}
    int operator * (const int & value){
        return numerator * value / denominator;
    }

    Fraction operator * (const Fraction & value){
        return Fraction(numerator * value.numerator, denominator * value.denominator);
    }
};

#endif