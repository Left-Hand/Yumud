#include "fconv.hpp"
#include "div.hpp"
#include <cmath>

using namespace ymd;
using namespace ymd::fxmath::details;

namespace {
#if 0
[[nodiscard]] static constexpr float _IQNtoF2(const int32_t iqNInput, size_t Q){
    //这段代码只有一处使用了Q, 不需要使用模板
    uint32_t uiq23_result_bits;
    uint32_t uiq31Input;

    /* Initialize exponent to the offset iq value. */
    uint32_t ui16Exp = static_cast<uint32_t>(0x3f80 + ((31u - Q) * (0x80)));

    /* Save the sign of the iqN input to the exponent construction. */
    if (iqNInput < 0) {
        ui16Exp |= 0x8000;
        if (iqNInput == std::numeric_limits<int32_t>::min()) [[unlikely]] {
            uiq31Input = 0x80000000;
        } else {
            uiq31Input = -iqNInput;
        }
    } else if (iqNInput == 0) {
        return (0);
    } else {
        uiq31Input = iqNInput;
    }

    const size_t leading_zeros = __builtin_clz(uiq31Input);
    uiq31Input <<= leading_zeros;
    ui16Exp -= (leading_zeros << 7);

    /* Right shift to uiq23 */
    uiq23_result_bits = uiq31Input >> 8;

    /* Remove the implied MSB bit of the mantissa. */
    uiq23_result_bits &= ~0x00800000;


    if (uiq23_result_bits == 0 && uiq31Input != 0) {
        // 尾数为0但值不为0，这意味着尾数是1.0
        // 指数已经正确，不需要调整
        // 但需要确保浮点数构造正确
    }

    /* Add the constructed exponent and sign bit to the mantissa. */
    uiq23_result_bits += ui16Exp << 16;

    /* Return as float. */
    return std::bit_cast<float>(uiq23_result_bits);
}
#else
[[nodiscard]] static constexpr float _IQNtoF2(const int32_t iqn_input, size_t Q){
    //这段代码只有一处使用了Q, 不需要使用模板
    uint32_t uiq23_result_bits;

    /* Initialize exponent to the offset iq value. */
    uint32_t ui16_exp = static_cast<uint32_t>(
        int32_t(0x3f80) + (int32_t(31 - int32_t(Q)) * (0x80)));

    auto conv_unsigned = [&](uint32_t uiqn_input) constexpr -> uint32_t {
        // return static_cast<uint32_t>(iqn_input) << (31 - Q);
        const size_t leading_zeros = __builtin_clz(uiqn_input);
        ui16_exp -= (leading_zeros << 7);
        return ((uiqn_input << leading_zeros) & 0x7fffffff) >> 8;
    };


    /* Save the sign of the iqN input to the exponent construction. */
    if (iqn_input < 0) {
        ui16_exp |= 0x8000;
        if (iqn_input == std::numeric_limits<int32_t>::min()) [[unlikely]] {
            uiq23_result_bits = 0;
        } else {
            uiq23_result_bits = conv_unsigned(std::bit_cast<uint32_t>(-iqn_input));
        }
    } else if (iqn_input == 0) {
        return std::bit_cast<float>(uint32_t(0));
    } else {
        uiq23_result_bits = conv_unsigned(std::bit_cast<uint32_t>(iqn_input));
    }


    /* Return as float. */
    return std::bit_cast<float>(uiq23_result_bits + (ui16_exp << 16));
}

#endif

}


namespace test_fconv{


[[maybe_unused]] void test_n_to_f(){ 

    {
        [[maybe_unused]] constexpr float f1 = _IQNtoF(std::numeric_limits<int32_t>::min(), 16);
        [[maybe_unused]] constexpr float f2 = _IQNtoF(std::numeric_limits<int32_t>::max(), 16);
        // INT_MIN 的负数溢出
        // 当 iqn_input = INT_MIN 时，-iqn_input 溢出
        static_assert(std::abs(f1 - (-32768.0f)) < 1E-4);

        [[maybe_unused]] constexpr float e2 = std::abs(f2 - (32768.0f));

        // TODO!!!
        // static_assert(e2 < ((1.1f) / (1u << 16)));
    }

    {
        [[maybe_unused]] constexpr float f1 = _IQNtoF(std::numeric_limits<int32_t>::min(), 31);
        [[maybe_unused]] constexpr float f2 = _IQNtoF(std::numeric_limits<int32_t>::max(), 31);
        static_assert(std::abs(f1 - (-1.0f)) < 1E-4);
        static_assert(std::abs(f2 - (1.0f)) < 1E-4);
        
        [[maybe_unused]] constexpr float f3 = _IQNtoF(-1,31);
        [[maybe_unused]] constexpr float f4 = _IQNtoF(1,31);

    }

    {
        constexpr float f1 = _IQNtoF(std::numeric_limits<int32_t>::min(), 32);
        constexpr float f2 = _IQNtoF(std::numeric_limits<int32_t>::max(), 32);
        static_assert(std::abs(f1 - (-0.5f)) < 1E-4);
        static_assert(std::abs(f2 - (0.5f)) < 1E-4);
    }

    {
        [[maybe_unused]] constexpr float f1 = _IQNtoF(0, 0);
        [[maybe_unused]] constexpr float f2 = _IQNtoF(1, 0);
        [[maybe_unused]] constexpr float f3 = _IQNtoF(90, 0);
        [[maybe_unused]] constexpr float f4 = _IQNtoF(900, 0);
        [[maybe_unused]] constexpr float f5 = _IQNtoF(9000, 0);
        [[maybe_unused]] constexpr float f6 = _IQNtoF(900000, 0);
        [[maybe_unused]] constexpr float f7 = _IQNtoF(int32_t(9E7), 0);
        [[maybe_unused]] constexpr float f8 = _IQNtoF(std::numeric_limits<int32_t>::max(), 0);

        [[maybe_unused]] constexpr float e8 = std::abs(f8 - (std::numeric_limits<int32_t>::max()));
        
        //TODO
        // static_assert(e8 < 1E-4);
    }

    {
        [[maybe_unused]] constexpr float f1 = _IQNtoF(0, 1);
        [[maybe_unused]] constexpr float f2 = _IQNtoF(1, 1);
        [[maybe_unused]] constexpr float f3 = _IQNtoF(90, 1);
        [[maybe_unused]] constexpr float f4 = _IQNtoF(900, 1);
        [[maybe_unused]] constexpr float f5 = _IQNtoF(9000, 1);
        [[maybe_unused]] constexpr float f6 = _IQNtoF(900000, 1);
        [[maybe_unused]] constexpr float f7 = _IQNtoF(int32_t(9E7), 1);
        [[maybe_unused]] constexpr float f8 = _IQNtoF(std::numeric_limits<int32_t>::max(), 1);

        //TODO
        // constexpr float e8 = std::abs(f8 - (std::numeric_limits<int32_t>::max()/2));
        // static_assert(e8 < 1E-4);
    }

    {
        // 最大正值边界
        // 检查最大正数的转换
        static_assert(std::abs(_IQNtoF(std::numeric_limits<int32_t>::max(), 16) - (static_cast<float>(std::numeric_limits<int32_t>::max()) / 65536.0f)) < 1,
                    "INT_MAX转换测试");

        // constexpr auto max_value = _IQNtoF(std::numeric_limits<int32_t>::max() / 2, 16);
        [[maybe_unused]] constexpr auto max_value = _IQNtoF(0x40000000, 16);
        [[maybe_unused]] constexpr auto max_value2 = _IQNtoF(std::numeric_limits<int32_t>::max(), 16);

        // 验证Q值范围
        // Q可能为0-31，但代码中的计算可能有问题
        // 特别是当Q=0或Q=31时的边界情况
        static_assert(_IQNtoF(1, 0) == 1.0f, "Q=0转换测试");
    }



    {
        // 测试舍入溢出
        // 找一個会使 uiq31_input + 0x0080 溢出的值
        // 需要 uiq31_input >= 0xFFFFFF80
        constexpr int32_t round_overflow_test = 0x7FFFFFFF;  // 在Q格式中
        // 当Q=0时，这对应很大的浮点数
        [[maybe_unused]] constexpr auto i = int64_t(_IQNtoF(round_overflow_test, 0));
        // static_assert( == round_overflow_test);
    }

    {
        constexpr int32_t large_for_q = 0x40000000;  // Q=1时对应0.5
        static_assert(_IQNtoF(large_for_q,31) == 0.5f, "大值指数调整测试");
    }

    {
        // 精度损失测试
        // 测试转换的对称性：f -> iq -> f 应该保持精度
        constexpr float original = 0.123456f;
        constexpr int32_t iq_val = _IQFtoN(original, 16);  // 假设有正向转换函数
        constexpr float recovered = _IQNtoF(iq_val, 16);
        static_assert(std::abs(original - recovered) < 3E-5, "往返转换精度测试");
    }

    {
        // 零值符号测试
        // 确保正零和负零都返回0.0f
        static_assert(_IQNtoF(0, 16) == 0.0f, "零值转换");
        static_assert(_IQNtoF(-0, 16) == 0.0f, "负零转换");
    }

    // 测试Q值极端情况
    // Q=31时，最小步长是 1/2^31 ≈ 4.6566e-10
    static_assert(_IQNtoF(1, 31) == 4.656612873077392578125e-10f, 
                "Q=31最小精度测试");

    //  验证指数构造的正确性
    // 检查指数部分的计算：0x3f80 + ((31 - Q) * ((uint32_t) 1 << (23 - 16)))
    // 0x3f80 = 127 << 7 (IEEE指数偏移127，但这里用16位存储)
    // 当Q=16时，指数部分应该是127 << 7 + (15 << 7) = 142 << 7
    constexpr float test_q16 = _IQNtoF(0x8000, 16);  // 应该得到0.5
    static_assert(test_q16 == 0.5f, "Q=16的0.5测试");



    {
        constexpr float f0 = _IQNtoF(0x8000, 16);
        constexpr float f1 = _IQNtoF(0x4000, 16);
        constexpr float f2 = _IQNtoF(0xC000, 16);

        [[maybe_unused]] constexpr float f3 = _IQNtoF(0x3FFFFFFF, 16);
        [[maybe_unused]] constexpr float f4 = _IQNtoF(0x40000000, 16);
        [[maybe_unused]] constexpr float f5 = _IQNtoF(0x40000101, 16);
        [[maybe_unused]] constexpr int32_t i5 = int32_t(_IQNtoF(0x40000010, 16) * (1u << 16));
        [[maybe_unused]] constexpr float f6 = _IQNtoF(0x7fffffff, 16);

        auto cmp_inc = [](const int32_t a, const int32_t b) -> bool{
            if(float(a) < float(b)){
                return _IQNtoF(a, 16) < _IQNtoF(b, 16);
            }else if(float(a) == float(b)){
                return true;
            }else{
                return false;
            }
        };
        // 测试基本功能
        static_assert(f0 == 0.5f, "0.5测试");
        static_assert(f1 == 0.25f, "0.25测试");
        static_assert(f2 == 0.75f, "0.75测试");
        static_assert(cmp_inc(0x40000000, 0x40000001));
        static_assert(cmp_inc(0x40000000, 0x40000101));
        static_assert(cmp_inc(0x40000000, 0x40110001));

    }

    // 测试特殊值
    static_assert(_IQNtoF(0, 16) == 0.0f, "零值测试");
    static_assert(_IQNtoF(std::numeric_limits<int32_t>::min(), 16) < 0.0f,
                "INT_MIN测试");
}

[[maybe_unused]] void test_f_to_n(){

    // 添加更多测试
    static_assert(_IQFtoN(0.0f, 16) == 0);
    static_assert(_IQFtoN(-0.0f, 16) == 0);
    static_assert(_IQFtoN(1.0f, 16) == 65536);  //: 1.0 = 65536
    static_assert(_IQFtoN(-1.0f, 16) == -65536);

    // 测试非常小的数
    static_assert(_IQFtoN(0.0001f, 16) == 6);  // 近似值
    static_assert(_IQFtoN(-0.0001f, 16) == -6);

    // 测试Q不同值的情况
    static_assert(_IQFtoN(1.0f, 8) == 256);    // Q8: 1.0 = 256
    static_assert(_IQFtoN(0.5f, 8) == 128);
    static_assert(_IQFtoN(1.0f, 24) == 16777216);  // Q24: 1.0 = 16777216

    static_assert(_IQFtoN(-32768.0f / 65536.0f, 16) == -32768);

    static_assert(_IQFtoN(6556, 16.0f) == 6556 << 16);
    static_assert(_IQFtoN(.04f, 16) == int32_t(0.04 * 65536));
    static_assert(_IQFtoN(.001f, 16) == int32_t(0.001 * 65536));

    static_assert(_IQFtoN(-6556.0f, 16) == -6556 * 65536);
    static_assert(_IQFtoN(-0.04f, 16) == int32_t(-0.04 * 65536));
    static_assert(_IQFtoN(-0.001f, 16) == int32_t(-0.001 * 65536));

    static_assert(_IQFtoN(66.0f, 24) == 66 << 24);
    static_assert(_IQFtoN(.04f, 24) == int32_t(0.04 * (1 << 24)));
    static_assert(_IQFtoN(.0001f, 24) == int32_t(0.0001 * (1 << 24)));

    static_assert(_IQFtoN(-66.0f, 24) == -66 * (1 << 24));
    static_assert(_IQFtoN(-0.04f, 24) == int32_t(-0.04 * (1 << 24)));
    static_assert(_IQFtoN(-0.0001f, 24) == int32_t(-0.0001 * (1 << 24)));

    static_assert(_IQFtoN(0.0f, 16) == 0);
    static_assert(_IQFtoN(-0.0f, 16) == 0);
    static_assert(_IQFtoN(1.0f, 16) == 65536);  // Q16: 1.0 = 65536
    static_assert(_IQFtoN(-1.0f, 16) == -65536);
    static_assert(_IQFtoN(0.5f, 16) == 32768);
    static_assert(_IQFtoN(-0.5f, 16) == -32768);
    static_assert(_IQFtoN(0.25f, 16) == 16384);
    static_assert(_IQFtoN(1.5f, 16) == 98304);  // 1.5 * 65536 = 98304

    // 测试非常小的数
    static_assert(_IQFtoN(0.0001f, 16) == 6);  // 近似值
    static_assert(_IQFtoN(-0.0001f, 16) == -6);

    // 测试Q不同值的情况
    static_assert(_IQFtoN(1.0f, 8) == 256);    // Q8: 1.0 = 256
    static_assert(_IQFtoN(0.5f, 8) == 128);
    static_assert(_IQFtoN(1.0f, 24) == 16777216);  // Q24: 1.0 = 16777216

    static_assert(_IQFtoN(-32768.0f / 65536.0f, 16) == -32768);



    static_assert(_IQFtoN(0, 16) == 0);
    static_assert(_IQFtoN(1E-9, 16) == 0);

    // 符号处理逻辑错误：-2.0在Q=1格式下应返回INT_MIN，但代码可能错误处理
    static_assert(_IQFtoN(-2.0f, 1) == std::numeric_limits<int32_t>::min(), "符号边界处理失败");

    // 溢出检测不充分：2.5在Q=30格式下应溢出到INT_MAX，但移位计算可能错误
    static_assert(_IQFtoN(2.5f, 30) == std::numeric_limits<int32_t>::max(), "正溢出检测失败");


    // 极小数处理：最小正非规格化数应返回0
    static_assert(_IQFtoN(std::numeric_limits<float>::denorm_min(), 16) == 0, "非规格化数处理失败");

    // 零值处理：正负零都应返回0
    static_assert(_IQFtoN(0.0f,8) == 0, "正零处理失败");
    static_assert(_IQFtoN(-0.0f,8) == 0, "负零处理失败");

    // 无穷大处理
    static_assert(_IQFtoN(std::numeric_limits<float>::infinity(), 16) == std::numeric_limits<int32_t>::max(), "正无穷处理失败");
    static_assert(_IQFtoN(-std::numeric_limits<float>::infinity(), 16) == std::numeric_limits<int32_t>::min(), "负无穷处理失败");

    // NaN处理
    static_assert(_IQFtoN(std::numeric_limits<float>::quiet_NaN(), 16) == 0, "NaN处理失败");

    // 精确值测试：1.0在Q=15格式下应为32768
    static_assert(_IQFtoN(1.0f,15) == 32768, "精确值转换失败");

    // 负精确值测试：-0.5在Q=15格式下应为-16384
    static_assert(_IQFtoN(-0.5f, 15) == -16384, "负精确值转换失败");

    // 过小值处理：极小的负数应返回0或INT_MIN
    static_assert(_IQFtoN(-1E-20f, 16) == 0 || _IQFtoN(-1E-20f, 16) == std::numeric_limits<int32_t>::min(), "极小负数处理失败");


    // 利用符号处理逻辑错误
    // 原始代码中：if (result == std::numeric_limits<int32_t>::min()) return min;
    // 但result是正数，永远不会等于INT_MIN，所以负数边界处理完全错误
    static_assert(_IQFtoN(-1.9999999f, 30) != std::numeric_limits<int32_t>::min(), 
                "符号处理逻辑错误应导致错误结果");

    // 溢出检测的边界条件
    // shift < -31 检查不够精确，应该考虑尾数的实际值
    // 例如 Q=0, f=1.0, exponent=0, shift=23-0-0=23 (右移)
    // 但如果是 Q=0, f=很大值，可能导致左移溢出
    constexpr float large_val = static_cast<float>((1ULL << 24) - 1);
    static_assert(_IQFtoN(large_val, 0) == std::numeric_limits<int32_t>::max(),
                "大值左移应检测溢出");


    // 左移溢出检测逻辑问题
    // 代码中的检查：if (mantissa_bits > (UINT32_MAX >> left_shift))
    // 但UINT32_MAX >> left_shift 可能为0，导致错误检测
    // 例如：left_shift=32时，UINT32_MAX>>32=0，任何mantissa_bits>0都会通过检测
    static_assert(_IQFtoN(2,31.0f) != 0, "左移32位处理可能有问题");

    // 非规格化数的指数计算
    // 原始代码对非规格化数直接返回0，但IEEE754非规格化数有特殊指数值
    constexpr float denorm = std::bit_cast<float>(0x00000001U); // 最小正非规格化数
    static_assert(_IQFtoN(denorm, 0) == 0, "非规格化数应返回0");

    // 检查符号位与取负的交互
    // 当is_negative=true且unsigned_result=0时，应该返回0而不是-0
    static_assert(_IQFtoN(-0.0000001f, 16) == 0, "极小负数应返回0而非-0");


    static_assert(_IQFtoN(1E-9, 30) == int32_t((1u << 30) * (1E-9)), "极小正数应正确处理");
    static_assert(_IQFtoN(1E-13, 30) == int32_t((1u << 30) * (1E-13)), "极小正数应正确处理");
    static_assert(_IQFtoN(1E-20, 30) == int32_t((1u << 30) * (1E-20)), "极小正数应正确处理");

    // 验证负零的符号处理
    // 负零在IEEE754中符号位为1，但值是0
    // 代码中is_negative=true，但最后会取负，得到-0，但转换为int32_t应该是0
    static_assert(_IQFtoN(0.0f, 8) == _IQFtoN(-0.0f, 8), "正负零应得到相同结果");
}

}

namespace test_div{
using namespace fxmath::details;


void test_div32u(){
    static_assert(div32u<1>(65536 << 1, 1 << 1) == 65536 << 1);
    static_assert(div32u<10>(65536 << 10, 1 << 10) == 65536 << 10);
    static_assert(div32u<10>(32768 << 10, 1 << 10) == 32768 << 10);
    static_assert(div32u<16>(32768 << 16, 1 << 16) == 32768u << 16);
    static_assert(div32u<16>(20000u << 16, 1u << 16) == 20000u << 16);
    static_assert(div32u<5>(32768 << 5, 1 << 5) == 32768 << 5);

    static_assert(div32u<0>(32768 << 0, 1 << 0) == 32768 << 0);
    // static_assert(div32u<32>(
    //     std::numeric_limits<uint32_t>::max(), 
    //     std::numeric_limits<uint32_t>::max()) 
    //     == std::numeric_limits<uint32_t>::max());
}

void test_div32i(){
    static_assert(div32i<1>(65536 << 1, -1 << 1) == -65536 << 1);
    static_assert(div32i<10>(65536 << 10, -1 << 10) == -65536 << 10);
    static_assert(div32i<10>(32768 << 10, -1 << 10) == -32768 << 10);
    static_assert(div32i<16>(20000 << 16, -1 << 16) == -20000 << 16);
    static_assert(div32i<5>(32768 << 5, -1 << 5) == -32768 << 5);

    static_assert(div32i<0>(32768 << 0, -1 << 0) == -32768 << 0);
    static_assert(div32i<31>(
        std::numeric_limits<int32_t>::max(), 
        std::numeric_limits<int32_t>::min()) 
        == std::numeric_limits<int32_t>::min());
}

}