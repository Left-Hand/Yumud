#include "../dsp_lpf.hpp"
#include "../dsp_vec.hpp"
#include "../dsp_pll.hpp"
#include "../dsp_pi.hpp"
#include "drivers/humiture/ntc/ntc.hpp"

using namespace ymd;
using namespace ymd::dsp;


template<typename Fn1, typename Fn2, typename ... Args>
static consteval bool is_result_nearly_equal(Fn1 && fn1, Fn2 && fn2, const long double eps, Args && ... args){
    const auto res1 = fn1(std::forward<Args>(args)...);
    const auto res2 = fn2(std::forward<Args>(args)...);
    return std::abs(static_cast<long double>(res1.unwrap()) - static_cast<long double>(res2.unwrap())) < eps;
}

template<std::floating_point T>
static constexpr T abs_err_percentages(const T dst, const T src){
    const auto abs_err = math::abs(dst - src);
    return static_cast<T>(abs_err * 100 / src);
}

namespace {

[[maybe_unused]] static void test_lpf_alpha_calc(){
    #define TEST_CASE(_fs, _fc, _eps)\
    static_assert(is_result_nearly_equal(\
        [](uint32_t fs, uint32_t fc){ return calc_lpf_alpha_f32(fs, fc); },\
        [](uint32_t fs, uint32_t fc){ return calc_lpf_alpha_uq32(fs, fc); },\
        _eps,\
        _fs, _fc\
    ));\


    TEST_CASE(900, 10, 1e-3)
    TEST_CASE(16000, 10, 1e-3)
    TEST_CASE(64000, 1400, 1e-3)
    TEST_CASE(32000, 20, 1e-3)
    TEST_CASE(32000, 200, 1e-3)
    TEST_CASE(34000, 1400, 1e-3)
    TEST_CASE(34000, 1200, 1e-3)

    #undef TEST_CASE
}



[[maybe_unused]] static void test_lpf_1o(){
    #define TEST_CASE(_type, _expect_y, _prev_y, _x, _alpha, _eps)\
    static_assert(abs_err_percentages(\
        (double)lpf_1o(_type(_prev_y), _type(_x), uq32(_alpha)), \
        double(_expect_y)) < _eps\
    );

    // 基本测试：步进响应
    TEST_CASE(iq16, 0.5, 0.0, 1.0, 0.5, 0.01)
    TEST_CASE(iq16, 0.3, 0.0, 0.6, 0.5, 0.01)
    TEST_CASE(iq20, 0.3, 0.0, 0.6, 0.5, 0.01)
    
    // 不同alpha值测试
    TEST_CASE(iq16, 0.25, 0.0, 1.0, 0.25, 0.01)
    TEST_CASE(iq16, 0.75, 0.0, 1.0, 0.75, 0.01)
    TEST_CASE(iq16, 0.1, 0.0, 1.0, 0.1, 0.01)
    
    // 非零初始状态测试
    TEST_CASE(iq16, 0.75, 0.5, 1.0, 0.5, 0.01)
    TEST_CASE(iq16, 0.55, 0.4, 0.7, 0.5, 0.01)
    TEST_CASE(iq16, 0.625, 0.5, 0.75, 0.5, 0.01)
    
    // 不同精度测试
    TEST_CASE(iq24, 0.5, 0.0, 1.0, 0.5, 0.001)
    TEST_CASE(iq24, 0.3, 0.0, 0.6, 0.5, 0.001)
    TEST_CASE(iq24, 0.75, 0.5, 1.0, 0.5, 0.001)
    
    // 极端值测试
    TEST_CASE(iq16, 0.99, 0.0, 1.0, 0.99, 0.01)
    TEST_CASE(iq16, 0.01, 0.0, 1.0, 0.01, 6e-2)
    // TEST_CASE(iq16, 0.0, 0.0, 0.0, 0.5, 0.01)  // x=0

    #undef TEST_CASE
}

// HPF测试用例
[[maybe_unused]] static void test_hpf_1o(){
    #define TEST_CASE(_type, _expect_y, _prev_y, _x, _x_prev, _alpha, _eps)\
    static_assert(abs_err_percentages(\
        (double)hpf_1o(_type(_prev_y), _type(_x), _type(_x_prev), uq32(_alpha)), \
        double(_expect_y)) < _eps\
    );

    // 基本测试：阶跃响应（HPF应该滤除直流分量）
    TEST_CASE(iq16, 0.5, 0.0, 1.0, 0.0, 0.5, 0.01)
    TEST_CASE(iq16, 0.25, 0.0, 1.0, 0.5, 0.5, 0.01)
    // TEST_CASE(iq16, 0.0, 0.0, 1.0, 1.0, 0.5, 6e-2)  // 输入不变，输出为0
    
    // 不同alpha值测试
    TEST_CASE(iq16, 0.25, 0.0, 1.0, 0.0, 0.25, 0.01)
    TEST_CASE(iq16, 0.75, 0.0, 1.0, 0.0, 0.75, 0.01)
    TEST_CASE(iq16, 0.1, 0.0, 1.0, 0.0, 0.1, 0.01)
    
    // 非零初始状态测试
    TEST_CASE(iq16, 0.75, 0.5, 1.0, 0.0, 0.5, 0.01)
    TEST_CASE(iq16, 0.5, 0.3, 0.7, 0.0, 0.5, 0.01)
    TEST_CASE(iq16, 0.625, 0.5, 0.75, 0.0, 0.5, 0.01)
    
    // 非零输入历史测试
    TEST_CASE(iq16, 0.35, 0.2, 0.9, 0.4, 0.5, 0.01)
    // TEST_CASE(iq16, 0.15, 0.1, 0.5, 0.2, 0.5, 0.01)
    
    // 不同精度测试
    TEST_CASE(iq24, 0.5, 0.0, 1.0, 0.0, 0.5, 0.001)
    TEST_CASE(iq24, 0.25, 0.0, 1.0, 0.5, 0.5, 0.001)
    TEST_CASE(iq24, 0.75, 0.5, 1.0, 0.0, 0.5, 0.001)
    
    // 极端值测试
    TEST_CASE(iq16, 0.99, 0.0, 1.0, 0.0, 0.99, 0.01)
    // TEST_CASE(iq16, 0.01, 0.0, 1.0, 0.0, 0.01, 0.01)
    // TEST_CASE(iq16, 0.0, 0.0, 1.0, 1.0, 0.99, 0.01)  // x == x_prev，输出应为0
    
    // 负值测试（如果支持有符号数）
    TEST_CASE(iq16, -0.25, 0.0, -0.5, 0.0, 0.5, 0.01)
    // TEST_CASE(iq16, 0.0, 0.0, -0.5, -0.5, 0.5, 0.01)

    #undef TEST_CASE
}



static_assert(dot2v2(1_iq20, 2_iq20, 3_iq20, 4_iq20) == 14_iq20);
static_assert(dot2v2(1_iq16, 2_iq16, 3_iq16, 4_iq16) == 14_iq16);
static_assert(cross2v2(
    1_iq20, 2_iq20, 
    3_iq20, 4_iq20) == -2_iq20);
static_assert(cross2v2(1_iq16, 2_iq16, 3_iq16, 4_iq16) == -2_iq16);



static_assert(std::abs((float)std::get<0>(resat_unit_circle(0.2_iq20, 0.0_iq20)) - 1.0f) < 1e-4);

static_assert(std::abs((float)std::get<0>(resat_unit_circle(1.0_iq20, 1.73205080757_iq20)) - 0.5f) < 1e-4);
static_assert(std::abs((float)std::get<1>(resat_unit_circle(1.0_iq20, 1.73205080757_iq20)) - 0.866025403784f) < 1e-4);

static_assert(std::abs((float)std::get<0>(resat_unit_circle(0.010_iq20, 0.0173205080757_iq20)) - 0.5f) < 1e-4);
static_assert(std::abs((float)std::get<1>(resat_unit_circle(0.010_iq20, 0.0173205080757_iq20)) - 0.866025403784f) < 1e-4);


static_assert(abs_err_percentages(
    double(TAU_SCALE_NUM) / TAU_SCALE_DEN, double(TAU)) < 3e-4);
static_assert(abs_err_percentages(
    double(INV_TAU_SCALE_NUM) / INV_TAU_SCALE_DEN, double(1.0 / TAU)) < 2e-2);




    // if(a > b){
    //     std::swap(a, b);
    //     ratio = 1 - ratio;
    // }
    // if(b - a > 0.5_uq32){
    //     return uq32((iq32(b) - iq32(a)) * ratio + iq32(a));
    // }else{
    //     return (b - a) * ratio + a;
    // }

// static_assert(abs_err_percentages((float)lerp_pu(0.5_uq32, 0.8_uq32, 0.1_uq32), 0.53f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.2_uq32, 0.4_uq32, 0.1_uq32), 0.22f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.2_uq32, 0.4_uq32, 0.9_uq32), 0.38f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.2_uq32, 0.9_uq32, 0.5_uq32), 0.05f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.2_uq32, 0.9_uq32, 0.2_uq32), 0.14f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.2_uq32, 0.1_uq32, 0.5_uq32), 0.15f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.6_uq32, 0.8_uq32, 0.5_uq32), 0.70f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.6_uq32, 0.8_uq32, 0.2_uq32), 0.64f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.9_uq32, 0.2_uq32, 0.5_uq32), 0.05f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.9_uq32, 0.2_uq32, 0.2_uq32), 0.96f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.8_uq32, 0.6_uq32, 0.5_uq32), 0.70f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.8_uq32, 0.6_uq32, 0.2_uq32), 0.76f) < 1e-3);

// static_assert(abs_err_percentages((float)lerp_pu(0.2_uq32, 0.8_uq32, 0.1_uq32), 0.16f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.2_uq32, 0.8_uq32, 0.9_uq32), 0.84f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.1_uq32, 0.9_uq32, 0.9_uq32), 0.92f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.1_uq32, 0.9_uq32, 0.1_uq32), 0.08f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.9_uq32, 0.1_uq32, 0.1_uq32), 0.92f) < 1e-3);
// static_assert(abs_err_percentages((float)lerp_pu(0.9_uq32, 0.1_uq32, 0.9_uq32), 0.08f) < 1e-3);




[[maybe_unused]] static void test_pll_coeff(){
    // kp = (zeta * 2 * fc);
    // ki_discrete = fc * fc / fs;

    #define TEST_CASE(_fs, _fc, _zeta)\
    {\
        static constexpr size_t fs = _fs;\
        static constexpr size_t fc = _fc;\
        static constexpr iq16 zeta = iq16(_zeta);\
        static constexpr auto coeffs = dsp::PllCoeffs::from_fsfc(fs, fc, zeta);\
        static constexpr auto err_kp = abs_err_percentages((float)coeffs.kp, (float)(float(zeta) * 2 * fc));\
        static constexpr auto err_ki_discrete = abs_err_percentages((float)coeffs.ki_discrete, (float)(float(fc) * fc / fs));\
        static_assert(err_kp < 1e-2);\
        static_assert(err_ki_discrete < 3e-2);\
    }\

    TEST_CASE(36000, 20, 1)
    TEST_CASE(36000, 40, 1)
    TEST_CASE(36000, 100, 1)
    TEST_CASE(36000, 100, 2)
    TEST_CASE(36000, 900, 1)

    #undef TEST_CASE
}

static_assert(abs_err_percentages(1.0f, 1.01f) > 0.9f);
static_assert(abs_err_percentages(1.0f, 1.01f) < 1.1f);

[[maybe_unused]] static void test_pi_calc(){
    // Ki = 2pi * fc * R
    // Kp = 2pi * fc * L
    #define TEST_CASE(_fs, _fc, _ind_mh, _res_ohm)\
    {\
        static constexpr size_t fs = _fs;\
        static constexpr size_t fc = _fc;\
        static constexpr auto CURRENT_REGULATOR_CFG = LrSeriesCurrentRegulatorConfig{\
            .fs = fs,\
            .fc = fc,\
            .phase_inductance_mh = iq20(_ind_mh),\
            .phase_resistance_ohm = iq20(_res_ohm),\
        };\
        static constexpr auto coeffs = CURRENT_REGULATOR_CFG.try_into_precomputed().unwrap();\
        static constexpr auto kp_err = abs_err_percentages((float)coeffs.kp, float(2 * M_PI * fc * _ind_mh * 0.001));\
        static constexpr auto ki_err = abs_err_percentages((float)coeffs.ki_discrete, float(2 * M_PI * fc * _res_ohm / fs));\
        static_assert(kp_err < 1e-2);\
        static_assert(ki_err < 1.5e-2);\
    }\


    TEST_CASE(36000, 1000, 0.222f, 0.222f)
    TEST_CASE(36000, 3000, 0.222f, 0.222f)
    TEST_CASE(36000, 200, 0.222f, 0.222f)

    TEST_CASE(46000, 1000, 0.222f, 0.222f)
    TEST_CASE(46000, 3000, 0.222f, 0.222f)
    TEST_CASE(46000, 200, 0.222f, 0.222f)

    TEST_CASE(16000, 1000, 0.222f, 0.222f)
    TEST_CASE(16000, 200, 0.222f, 0.222f)

    TEST_CASE(16000, 1000, 0.222f, 9.222f)
    TEST_CASE(16000, 200, 0.222f, 9.222f)

    TEST_CASE(36000, 1000, 0.022f, 0.222f)
    TEST_CASE(36000, 3000, 0.022f, 0.222f)
    TEST_CASE(36000, 200, 0.022f, 0.222f)

    TEST_CASE(36000, 1000, 0.022f, 0.072f)
    TEST_CASE(36000, 3000, 0.022f, 0.072f)
    TEST_CASE(36000, 200, 0.022f, 0.072f)

    TEST_CASE(56000, 1000, 0.022f, 0.072f)
    TEST_CASE(56000, 3000, 0.022f, 0.072f)
    TEST_CASE(56000, 200, 0.022f, 0.072f)

    TEST_CASE(32768, 1000, 0.022f, 0.222f)
    TEST_CASE(32768, 3000, 0.022f, 0.222f)
    TEST_CASE(32768, 200, 0.022f, 0.222f)

    TEST_CASE(36000, 1000, 0.022f, 1.222f)
    TEST_CASE(36000, 3000, 0.022f, 1.222f)
    TEST_CASE(36000, 200, 0.022f, 1.222f)

    TEST_CASE(36000, 1000, 1.022f, 9.222f)
    TEST_CASE(36000, 3000, 1.022f, 9.222f)
    TEST_CASE(36000, 200, 1.022f, 9.222f)

    #undef TEST_CASE
}


[[maybe_unused]] static void test_invtable_int(){

    {
        static constexpr size_t LG2_TABLE_LEN = 2;
        static constexpr size_t TABLE_LEN = 1 << LG2_TABLE_LEN;
        static constexpr int32_t TABLE[TABLE_LEN] = {4,5,7,8};

        static constexpr auto LEN_INFO = TableLengthInfo::from_len(TABLE_LEN);

        static_assert(LEN_INFO.len == 4);
        static_assert(LEN_INFO.leading_zeros == 30);


        static_assert(invlerp_table_roundback(TABLE, LEN_INFO, 0.25_uq32) == 5);
        static_assert(invlerp_table_roundback(TABLE, LEN_INFO, 0.375_uq32) == 6);
        static_assert(invlerp_table_roundback(TABLE, LEN_INFO, 0.5_uq32) == 7);
        static_assert(invlerp_table_roundback(TABLE, LEN_INFO, 0.75_uq32) == 8);
        static_assert(invlerp_table_roundback(TABLE, LEN_INFO, 0.875_uq32) == 6);

        static_assert(pu_ratio_to_nearest_roundback_index(LEN_INFO, 0.124_uq32) == 0);
        static_assert(pu_ratio_to_nearest_roundback_index(LEN_INFO, 0.126_uq32) == 1);
        static_assert(pu_ratio_to_nearest_roundback_index(LEN_INFO, 0.874_uq32) == 3);
        static_assert(pu_ratio_to_nearest_roundback_index(LEN_INFO, 0.876_uq32) == 0);
    }

    {
        static constexpr size_t LG2_TABLE_LEN = 2;
        static constexpr size_t TABLE_LEN = 1 << LG2_TABLE_LEN;
        static constexpr iq16 TABLE[TABLE_LEN] = {4,5,7,8};

        static constexpr auto LEN_INFO = TableLengthInfo::from_len(TABLE_LEN);

        static_assert(LEN_INFO.len == 4);
        static_assert(LEN_INFO.leading_zeros == 30);


        static_assert(invlerp_table_roundback(TABLE, LEN_INFO, 0.25_uq32).to_bits() == 5 << 16);
        static_assert(invlerp_table_roundback(TABLE, LEN_INFO, 0.375_uq32).to_bits() == 6 << 16);
        static_assert(invlerp_table_roundback(TABLE, LEN_INFO, 0.5_uq32).to_bits() == 7 << 16);
        static_assert(invlerp_table_roundback(TABLE, LEN_INFO, 0.75_uq32).to_bits() == 8 << 16);
        static_assert(invlerp_table_roundback(TABLE, LEN_INFO, 0.875_uq32).to_bits() == 6 << 16);

        static_assert(pu_ratio_to_nearest_roundback_index(LEN_INFO, 0.124_uq32) == 0);
        static_assert(pu_ratio_to_nearest_roundback_index(LEN_INFO, 0.126_uq32) == 1);
        static_assert(pu_ratio_to_nearest_roundback_index(LEN_INFO, 0.874_uq32) == 3);
        static_assert(pu_ratio_to_nearest_roundback_index(LEN_INFO, 0.876_uq32) == 0);
    }

    {
        static constexpr size_t TABLE_LEN = 4;
        static constexpr int32_t TABLE[TABLE_LEN] = {4,5,7,8};

        static constexpr auto LEN_INFO = TableLengthInfo::from_len(TABLE_LEN);

        static_assert(invlerp_table_nonround(TABLE, LEN_INFO, 0.25_uq32) == 5);
        static_assert(invlerp_table_nonround(TABLE, LEN_INFO, 0.375_uq32) == 6);
        static_assert(invlerp_table_nonround(TABLE, LEN_INFO, 0.5_uq32) == 7);
        static_assert(invlerp_table_nonround(TABLE, LEN_INFO, 0.75_uq32) == 8);
        static_assert(invlerp_table_nonround(TABLE, LEN_INFO, 0.875_uq32) == 8);
    }

}



// Apache 2.0
// https://github.com/mjbots/moteus/tree/main/fw/thermistor.h
// 32点插值表计算温度
// 经过py脚本(ntc_plot.py)对比可发现在-40°C~125°C内该方法基本贴合数值计算方法，而工业级的温度需求是-40°C至+85°C
// 前提是桥臂另一侧定值电阻和R0较为接近，即25°C时位于中性点位

static constexpr size_t THERMISTOR_TABLE_LEN = 32;

template<typename D>
static constexpr void init_thermistor_lookup_table(
    std::span<D, THERMISTOR_TABLE_LEN> table,
    const drivers::NtcCalculatorF & ntc_calc,
    const float rdiv_kohms // The resistor divider pair.
){
    for(size_t i = 1; i < THERMISTOR_TABLE_LEN; i++){
        const float norm_v = float(i) / THERMISTOR_TABLE_LEN;
        float rt_kohms = rdiv_kohms / norm_v - rdiv_kohms;
        table[i] = static_cast<D>(ntc_calc.kohms_to_celsius(rt_kohms));
    }
    table[0] = table[1];
}

template<typename D>
static constexpr D invlerp_thermistor_table(
    std::span<const D, THERMISTOR_TABLE_LEN> table, 
    uint16_t adc_raw
) {
    constexpr auto LEN_INFO = TableLengthInfo::from_len(THERMISTOR_TABLE_LEN);

    constexpr size_t ADC_SHIFT = 20;//32 - log2(4096)

    const uq32 pu_ratio = uq32::from_bits(uint32_t(adc_raw) << ADC_SHIFT);
    return invlerp_table_nonround(table.data(), LEN_INFO, pu_ratio);
}


[[maybe_unused]] static void test_thermistor_table(){
    {
        static constexpr auto ntc = drivers::NtcCalculatorF::from_b0r0(3950, 100);

        using D = int32_t;
        static constexpr auto thermistor_table = []{
            std::array<D, THERMISTOR_TABLE_LEN> table;
            init_thermistor_lookup_table<D>(std::span(table), ntc, 100);
            return table;
        }();

        static constexpr auto c25 = invlerp_thermistor_table(std::span(thermistor_table), 2048);
        static_assert((float)c25 == 25.0f);
    }
}






// // B参数转Steinhart-Hart
// static constexpr SteinhartHartParams b0r0_to_abc(const float b0, const float r0){
//     constexpr float T0 = 298.15f;
//     {
//         1.0f/T0 - math::ln(r0)/b0,
//         1.0f/b0,
//         0.0f
//     };
// }

// struct SteinhartHartNTC {
//     float A, B, C;  // Steinhart-Hart系数
    
//     // 从B参数转换
//     static SteinhartHartNTC from_B_R0(float B, float R0) {

//     }
    
//     float resistance_to_celsius(float R) const {
//         float lnR = logf(R);
//         float invT = A + B * lnR + C * lnR * lnR * lnR;
//         return 1.0f/invT - 273.15f;
//     }
// };

}

