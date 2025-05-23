#include "../tb.h"

#include "core/math/int/int_t.hpp"
#include "core/debug/debug.hpp"

#include "core/math/real.hpp"

#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;
using namespace ymd::hal;

template<int Q>
void iqmath_tb() {
    // LambdaPool pool;
    // pool.addLambda([](int a, double b) { std::cout << "Called with int and double: " << a << ", " << b << std::endl; });
    // pool.addLambda([](std::string s) { std::cout << "Called with string: " << s << std::endl; });

    // pool.executeAll(10, 20.5); // 调用第一个Lambda
    // pool.executeAll("hello"); // 调用第二个Lambda

    // DEBUG_VALUE(cos(iq_t(1)));
    // DEBUG_VALUE(iq_t(1).value);
    // DEBUG_VALUE(iq_t(2).value);
    // DEBUG_VALUE((iq_t(2) * iq_t(1)).value);
    // DEBUG_VALUE(((_IQ(1) * _IQ(2)) >> GLOBAL_Q));
    // DEBUG_VALUE(sqrt(iq_t(2)));
    // DEBUG_VALUE(sqrt(iq_t(1.2)));
    // DEBUG_VALUE(log(iq_t(2)));
    // DEBUG_VALUE(atan(iq_t(2)));
    // DEBUG_VALUE(atan(iq_t(1)));
    // DEBUG_VALUE(atan(iq_t(1.01)));
    // DEBUG_VALUE(atan(iq_t(1.99)));
    // DEBUG_VALUE(tan(iq_t(M_PI/2 - 0.01)));
    // DEBUG_VALUE(atan(iq_t(0)));
    // DEBUG_VALUE(atan(iq_t(0.01)));
    // DEBUG_VALUE(atan(iq_t(0.5)));
    // DEBUG_VALUE(atan(iq_t(2)));

    // DEBUG_VALUE(_IQatan2(_IQ(1), _IQ(2)) * (1.0 / (1 << GLOBAL_Q)));
    // DEBUG_VALUE(acos(iq_t(0.2)));
    // DEBUG_VALUE(fmod(iq_t(0.2), iq_t(0.12)));
    // DEBUG_VALUE(lerp(iq_t(0.2), iq_t(0), iq_t(1)));

    
    // DEBUG_VALUE(pow(iq_t(1.5), iq_t(2)))
    // DEBUG_VALUE(iq_t(2) * log(iq_t(1.5)))
    // DEBUG_VALUE(iq_t(1) * iq_t(2))
    // DEBUG_VALUE(iq_t(1.5) * 2)
    // DEBUG_VALUE(exp(iq_t(2)));
    // DEBUG_VALUE(log(iq_t(1.5)));
    // DEBUG_VALUE(_IQlog(_IQ(1.5)));
    // DEBUG_VALUE(iq_t(1.5).value);
    // DEBUG_VALUE(iq_t(2).value);
    // DEBUG_VALUE(log(iq_t(2)));
    
    // #define DEBUG_IQ_BASICS

    // #ifdef DEBUG_IQ_BASICS
    // DEBUG_VALUE((iq_t(M_PI)));
    // DEBUG_VALUE((iq_t(M_PI)/iq_t(2)));
    // DEBUG_VALUE((iq_t(M_PI)/2));
    // DEBUG_VALUE((iq_t(M_PI)*iq_t(2)));
    // DEBUG_VALUE((iq_t(M_PI)*2));
    // DEBUG_VALUE(sqrt(iq_t(3))/2);
    // DEBUG_VALUE(sqrt(3)/2);
    // DEBUG_VALUE(fmod(iq_t(0.3), iq_t(0.2)));

    // #endif

    // #define DEBUG_IQ_SINCOS
    // #ifdef DEBUG_IQ_SINCOS
    // DEBUG_VALUE(sin(iq_t(M_PI/3)));
    // DEBUG_VALUE(cos(iq_t(M_PI/3)));
    
    // DEBUG_VALUE(acos(iq_t(sqrt(iq_t(3))/2)) / M_PI);
    // DEBUG_VALUE(acos(iq_t(sqrt(iq_t(2))/2)) / M_PI);
    // #endif
    
    // // #define DEBUG_IQ_TAN
    // #ifdef DEBUG_IQ_TAN
    // DEBUG_VALUE(atan(iq_t(sqrt(iq_t(3)))) / M_PI);
    // #endif

    // #define DEBUG_IQ_LOG
    // //FIXME
    // #ifdef DEBUG_IQ_LOG
    // DEBUG_VALUE(log(iq_t(1)));
    // DEBUG_VALUE(log(iq_t(1.29)));
    // DEBUG_VALUE(log(iq_t(1.49)));
    // DEBUG_VALUE(log(iq_t(1.51)));
    // DEBUG_VALUE(log(iq_t(2)));
    // DEBUG_VALUE(log(iq_t(6)));

    // #endif
    
    // #define DEBUG_IQ_EXP
    // #ifdef DEBUG_IQ_EXP
    // DEBUG_VALUE(exp(iq_t(1)));
    // DEBUG_VALUE(exp(iq_t(1.5)));
    // DEBUG_VALUE(exp(iq_t(2)));
    // DEBUG_VALUE(exp(iq_t(6)));

    // #endif

    // DEBUG_VALUE(atan2(iq_t(1), iq_t(2)));
    // DEBUG_VALUE(sqrt(iq_t(2)));

    #define DEBUG_IQ_BASICS

    // auto a = std::is_arithmetic_v<iq_t<16>>;
    // DEBUG_VALUE(a);
    #ifdef DEBUG_IQ_BASICS

    // auto a = (iq_t<16>(M_PI));
    // if constexpr(std::is_arithmetic_v<decltype(a)>){
        // DEBUGGER << a;
    // }else{
    //     // DEBUGGER << a;
    // }

    auto a = iq_t<16>(2.5);
    // a -= iq_t<16>(2/4);
    auto b = iq_t<15>(12.5);
    // a -= iq_t<16>(b);
    DEBUG_VALUE(a);
    DEBUG_VALUE(b);
    DEBUG_VALUE(a + b);
    DEBUG_VALUE(a - b);
    DEBUG_VALUE(a * b);
    DEBUG_VALUE(int(a));
    DEBUG_VALUE(uint8_t(a));
    DEBUG_VALUE(a == b - iq_t<16>(10));
    
    // print_q(b);

    DEBUG_VALUE((iq_t<16>(PI)/iq_t<16>(2)));
    DEBUG_VALUE((iq_t<16>(PI)/2));
    DEBUG_VALUE((iq_t<16>(PI)*iq_t<16>(2)));
    DEBUG_VALUE((iq_t<16>(PI)*2));
    // DEBUG_VALUE(sqrt(iq_t<16>(3))/2);
    // DEBUG_VALUE(sqrt(3)/2);
    // DEBUG_VALUE(fmod(iq_t<16>(0.3), iq_t<16>(0.2)));

    #endif
}
