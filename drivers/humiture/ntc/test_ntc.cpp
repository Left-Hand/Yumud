#include "ntc.hpp"
#include <span>


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

static constexpr float cubic(const float x){
    return x * x * x;
}

struct [[nodiscard]] SteinhartHartParams final{
    float a;
    float b;
    float c;

    constexpr float ohms_to_invkelvin(const float r) const {
        const auto lnr = math::ln(r);
        return a + b * lnr + c * cubic(lnr);
    }

    // https://blog.csdn.net/xiaoyuanwuhui/article/details/133962276

    constexpr float invkelvin_to_ohms(const float inv_t) const {
        const auto x = (1 / c) * (a - inv_t);
        return math::sqrt(cubic(b / (3 * c)) + (x * x) * 0.25f);
    }
};


#if 1
struct Rtpair{
    float ohms;
    float celsius;
};

static constexpr SteinhartHartParams fitting_sh_params(
    const Rtpair p0,
    const Rtpair p1,
    const Rtpair p2
) {
    // 1. 数据准备：将温度转换为开尔文，电阻取自然对数
    const float inv_t0 = 1.0f / (p0.celsius + 273.15f);
    const float inv_t1 = 1.0f / (p1.celsius + 273.15f);
    const float inv_t2 = 1.0f / (p2.celsius + 273.15f);

    const float lnr0 = std::log(p0.ohms);
    const float lnr1 = std::log(p1.ohms);
    const float lnr2 = std::log(p2.ohms);

    const float x0 = lnr0, x1 = lnr1, x2 = lnr2;
    const float y0 = inv_t0, y1 = inv_t1, y2 = inv_t2;
    const float x0_3 = x0 * x0 * x0;
    const float x1_3 = x1 * x1 * x1;
    const float x2_3 = x2 * x2 * x2;

    // 2. 系数矩阵：
    //   [ 1,  x0,  x0_3 ]   [a]   [y0]
    //   [ 1,  x1,  x1_3 ] * [b] = [y1]
    //   [ 1,  x2,  x2_3 ]   [c]   [y2]

    // 计算行列式 det（系数矩阵）
    const float det = (x1 * x2_3 - x2 * x1_3)
                    - (x0 * x2_3 - x2 * x0_3)
                    + (x0 * x1_3 - x1 * x0_3);

    // 计算 det_a（替换第一列）
    const float det_a = (y0 * (x1 * x2_3 - x2 * x1_3))
                      - (x0 * (y1 * x2_3 - y2 * x1_3))
                      + (x0_3 * (y1 * x2 - y2 * x1));

    // 计算 det_b（替换第二列）
    const float det_b = y0 * (x1_3 - x2_3)
                      + y1 * (x2_3 - x0_3)
                      + y2 * (x0_3 - x1_3);

    // 计算 det_c（替换第三列）
    const float det_c = y0 * (x2 - x1)
                      + y1 * (x0 - x2)
                      + y2 * (x1 - x0);

    // 3. 求得系数
    SteinhartHartParams result;
    result.a = det_a / det;
    result.b = det_b / det;
    result.c = det_c / det;

    return result;
}
#endif





__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr int32_t lerp_i32_uq32(
    const int32_t a, 
    const int32_t b, 
    const uq32 ratio
){
    return a + intrinsics::mul32hsu(b - a, ratio.to_bits());
}

__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr int32_t lerp_u32_uq32(
    const uint32_t a, 
    const uint32_t b, 
    const uq32 ratio
){
    return a + intrinsics::mul32hu(b - a, ratio.to_bits());
}


template<typename D>
requires(sizeof(D) <= 4)
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr D lerp_int_uq32(
    const D a, const D b, const uq32 ratio
){
    if constexpr(std::is_signed_v<D>){
        return lerp_i32_uq32(static_cast<int32_t>(a), static_cast<int32_t>(b), ratio);
    }else{
        return lerp_u32_uq32(static_cast<uint32_t>(a), static_cast<uint32_t>(b), ratio);
    }
}



struct alignas(4) [[nodiscard]] FracIndex final{
    uq32 frac;
    uint32_t index;
};

struct alignas(4) [[nodiscard]] TableLengthInfo final{
    uint32_t len;
    uint32_t lg2_len;

    static constexpr TableLengthInfo from(size_t len){
        if(not std::has_single_bit(len)) __builtin_trap();
        return TableLengthInfo{
            .len = len,
            .lg2_len = uint32_t(__builtin_clz(len - 1))
        };
    }
};

__always_inline __attribute__((const, optimize( "-Ofast" )))
constexpr FracIndex pu_ratio_to_fracindex(TableLengthInfo len_info, const uq32 ratio) {
    const uint32_t index = (ratio.to_bits() >> (len_info.lg2_len));
    const uq32 frac = ratio * len_info.len;
    return {frac, index};
}

__always_inline __attribute__((const, optimize( "-Ofast" )))
[[nodiscard]] constexpr uint32_t 
pu_ratio_to_nearest_roundback_index(TableLengthInfo len_info, const uq32 ratio) {
    const uint32_t index = (ratio.to_bits() >> (len_info.lg2_len));
    const uq32 frac = ratio * len_info.len;

    //uppround
    return (index + bool(frac.to_bits() >> 31)) & (len_info.len - 1);
}

template<typename D>
requires(std::is_integral_v<D>)
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr D 
invlerp_table_roundback(const D * table_data, const TableLengthInfo len_info, const uq32 ratio){
    const uint32_t index = (ratio.to_bits() >> (len_info.lg2_len));
    const uq32 frac = ratio * len_info.len;

    const D value = table_data[index];

    const uint32_t next_index = (index + 1) & (len_info.len - 1);
    const D next_value = table_data[next_index];

    return lerp_int_uq32(value, next_value, frac);
}

template<size_t Q, typename D>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q, D> 
invlerp_table_roundback(const math::fixed<Q, D> * table_data, const TableLengthInfo len_info, const uq32 ratio){
    return invlerp_table_roundback<D>(&table_data->bits, len_info,ratio);
}


template<typename D>
requires(std::is_integral_v<D>)
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr D 
invlerp_table_nonround(const D * table_data, const TableLengthInfo len_info, const uq32 ratio){
    const uint32_t index = (ratio.to_bits() >> (len_info.lg2_len));
    const uq32 frac = ratio * len_info.len;
    
    const D value = table_data[index];

    uint32_t next_index = (index + 1);
    if((next_index) >= len_info.len) return value;

    const D next_value = table_data[next_index];
    return lerp_int_uq32(value, next_value, frac);
}

template<size_t Q, typename D>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q, D> 
invlerp_table_nonround(const math::fixed<Q, D> * table_data, const TableLengthInfo len_info, const uq32 ratio){
    return invlerp_table_nonround<D>(&table_data->bits, len_info,ratio);
}




[[maybe_unused]] static void test_invtable_int(){

    {
        static constexpr size_t TABLE_SIZE = 4;
        static constexpr int32_t TABLE[TABLE_SIZE] = {4,5,7,8};

        static constexpr auto LEN_INFO = TableLengthInfo::from(TABLE_SIZE);

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
        static constexpr size_t TABLE_SIZE = 4;
        static constexpr int32_t TABLE[TABLE_SIZE] = {4,5,7,8};

        static constexpr auto LEN_INFO = TableLengthInfo::from(TABLE_SIZE);

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

static constexpr size_t THERMISTOR_TABLE_SIZE = 32;

template<typename D>
static constexpr void init_thermistor_lookup_table(
    std::span<D, THERMISTOR_TABLE_SIZE> table,
    const NtcCalculatorF & ntc_calc,
    const float rdiv_kohms // The resistor divider pair.
){
    for(size_t i = 1; i < THERMISTOR_TABLE_SIZE; i++){
        const float norm_v = float(i) / THERMISTOR_TABLE_SIZE;
        float rt_kohms = rdiv_kohms / norm_v - rdiv_kohms;
        table[i] = static_cast<D>(ntc_calc.kohms_to_celsius(rt_kohms));
    }
    table[0] = table[1];
}

template<typename D>
static constexpr D invlerp_thermistor_table(
    std::span<const D, THERMISTOR_TABLE_SIZE> table, 
    uint16_t adc_raw
) {
    constexpr auto LEN_INFO = TableLengthInfo::from(THERMISTOR_TABLE_SIZE);

    constexpr size_t ADC_SHIFT = 20;//32 - log2(4096)

    const uq32 pu_ratio = uq32::from_bits(uint32_t(adc_raw) << ADC_SHIFT);
    return invlerp_table_nonround(table.data(), LEN_INFO, pu_ratio);
}


[[maybe_unused]] static void test_thermistor_table(){
    {
        static constexpr auto ntc = drivers::NtcCalculatorF::from_b0r0(3950, 100);

        using D = int32_t;
        static constexpr auto thermistor_table = []{
            std::array<D, THERMISTOR_TABLE_SIZE> table;
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