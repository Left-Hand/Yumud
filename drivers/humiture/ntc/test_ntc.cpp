#include "ntc.hpp"


using namespace ymd;
using namespace ymd::drivers;

namespace{
template<typename T>
static constexpr T abs_err(const T a, const T b){
    return (a > b) ? (a - b) : (b - a);
}
    
[[maybe_unused]] static void test_ntc(){

    #define TEST_CASE(kohms, celsius, eps)\
    static_assert(abs_err(ntc.kohms_to_celsius(iq16(kohms)), iq16(celsius)).to_bits() < iq16(eps).to_bits());\
    

    {
        static constexpr auto ntc = drivers::NtcCalculator::from_b0r0(3950, 100_uq16);
        
        // 使用 B参数公式, 在宽温度范围内误差会增大
        // 参考数据来源: MF52-104F3950 NTC 阻值表 [citation:3]
        TEST_CASE(948.8f,  -20.0f, 10.0f);   // 低温区误差较大
        TEST_CASE(548.1f,  -10.0f, 5.0f);
        TEST_CASE(194.0f,   10.0f, 4.0f);
        TEST_CASE(125.2f,   20.0f, 3.0f);
        TEST_CASE(100.0f,   25.0f, 3.1f);   // 基准点, 几乎无误差
        TEST_CASE(80.39f,   30.0f, 3.5f);
        TEST_CASE(52.93f,   40.0f, 2.0f);
        TEST_CASE(35.70f,   50.0f, 3.5f);
        TEST_CASE(6.40f,   100.0f, 6.0f);  // 高温区误差较大
    }

    {
        static constexpr auto ntc = drivers::NtcCalculator::from_b0r0(3950, 10_uq16);

        // 参考数据来源: 敏创电子 10K 3950 NTC R/T表 [citation:9]
        // 温度(°C) | 标称阻值(kΩ)
        TEST_CASE(97.0264f, -20.0f, 3.0f);  // 低温区间误差会稍大
        TEST_CASE(55.2491f, -10.0f, 3.0f);
        TEST_CASE(32.6142f,   0.0f, 3.5f);
        TEST_CASE(19.8884f,  10.0f, 3.0f);
        TEST_CASE(12.4905f,  20.0f, 3.5f);
        TEST_CASE(10.0000f,  25.0f, 3.1f);  // 基准点, 误差应该最小
        TEST_CASE(8.0574f,   30.0f, 3.5f);
        TEST_CASE(5.3266f,   40.0f, 3.0f);
        TEST_CASE(3.6012f,   50.0f, 1.5f);
        TEST_CASE(2.4856f,   60.0f, 3.0f);
        TEST_CASE(1.4767f,   75.0f, 3.0f);  // 高温区间误差也会变大
    }

    #undef TEST_CASE
}

[[maybe_unused]] static void test_ntc_f(){

    #define TEST_CASE(kohms, celsius, eps)\
    static_assert(abs_err(ntc.kohms_to_celsius(float(kohms)), float(celsius)) < float(eps));\
    

    {
        static constexpr auto ntc = drivers::NtcCalculatorF::from_b0r0(3950, 100);
        
        // 使用 B参数公式, 在宽温度范围内误差会增大
        // 参考数据来源: MF52-104F3950 NTC 阻值表 [citation:3]
        TEST_CASE(948.8f,  -20.0f, 10.0f);   // 低温区误差较大
        TEST_CASE(548.1f,  -10.0f, 5.0f);
        TEST_CASE(194.0f,   10.0f, 2.0f);
        TEST_CASE(125.2f,   20.0f, 1.0f);
        TEST_CASE(100.0f,   25.0f, 0.1f);   // 基准点, 几乎无误差
        TEST_CASE(80.39f,   30.0f, 0.5f);
        TEST_CASE(52.93f,   40.0f, 1.0f);
        TEST_CASE(35.70f,   50.0f, 1.5f);
        TEST_CASE(6.40f,   100.0f, 5.0f);  // 高温区误差较大
    }

    {
        static constexpr auto ntc = drivers::NtcCalculatorF::from_b0r0(3950, 10.0f);

        // 参考数据来源: 敏创电子 10K 3950 NTC R/T表 [citation:9]
        // 温度(°C) | 标称阻值(kΩ)
        TEST_CASE(97.0264f, -20.0f, 3.0f);  // 低温区间误差会稍大
        TEST_CASE(55.2491f, -10.0f, 2.0f);
        TEST_CASE(32.6142f,   0.0f, 1.5f);
        TEST_CASE(19.8884f,  10.0f, 1.0f);
        TEST_CASE(12.4905f,  20.0f, 0.5f);
        TEST_CASE(10.0000f,  25.0f, 0.1f);  // 基准点, 误差应该最小
        TEST_CASE(8.0574f,   30.0f, 0.5f);
        TEST_CASE(5.3266f,   40.0f, 1.0f);
        TEST_CASE(3.6012f,   50.0f, 1.5f);
        TEST_CASE(2.4856f,   60.0f, 2.0f);
        TEST_CASE(1.4767f,   75.0f, 3.0f);  // 高温区间误差也会变大
    }


    #undef TEST_CASE
}
}