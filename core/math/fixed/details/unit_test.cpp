#include "_IQNconv.hpp"
#include <cmath>

using namespace ymd;
using namespace ymd::fxmath::details;


namespace {

[[maybe_unused]] void test_n_to_f(){ 

    {
        [[maybe_unused]] constexpr float f1 = _IQNtoF<16>(std::numeric_limits<int32_t>::min());
        [[maybe_unused]] constexpr float f2 = _IQNtoF<16>(std::numeric_limits<int32_t>::max());
        // INT_MIN 的负数溢出
        // 当 iqNInput = INT_MIN 时，-iqNInput 溢出
        static_assert(std::abs(f1 - (-32768.0f)) < 1E-4);

        [[maybe_unused]] constexpr float e2 = std::abs(f2 - (32768.0f));

        // TODO!!!
        // static_assert(e2 < ((1.1f) / (1u << 16)));
    }

    {
        [[maybe_unused]] constexpr float f1 = _IQNtoF<31>(std::numeric_limits<int32_t>::min());
        [[maybe_unused]] constexpr float f2 = _IQNtoF<31>(std::numeric_limits<int32_t>::max());
        static_assert(std::abs(f1 - (-1.0f)) < 1E-4);
        static_assert(std::abs(f2 - (1.0f)) < 1E-4);
        
        [[maybe_unused]] constexpr float f3 = _IQNtoF<31>(-1);
        [[maybe_unused]] constexpr float f4 = _IQNtoF<31>(1);

    }

    {
        constexpr float f1 = _IQNtoF<32>(std::numeric_limits<int32_t>::min());
        constexpr float f2 = _IQNtoF<32>(std::numeric_limits<int32_t>::max());
        static_assert(std::abs(f1 - (-0.5f)) < 1E-4);
        static_assert(std::abs(f2 - (0.5f)) < 1E-4);
    }

    {
        [[maybe_unused]] constexpr float f1 = _IQNtoF<0>(0);
        [[maybe_unused]] constexpr float f2 = _IQNtoF<0>(1);
        [[maybe_unused]] constexpr float f3 = _IQNtoF<0>(90);
        [[maybe_unused]] constexpr float f4 = _IQNtoF<0>(900);
        [[maybe_unused]] constexpr float f5 = _IQNtoF<0>(9000);
        [[maybe_unused]] constexpr float f6 = _IQNtoF<0>(900000);
        [[maybe_unused]] constexpr float f7 = _IQNtoF<0>(int32_t(9E7));
        [[maybe_unused]] constexpr float f8 = _IQNtoF<0>(std::numeric_limits<int32_t>::max());

        [[maybe_unused]] constexpr float e8 = std::abs(f8 - (std::numeric_limits<int32_t>::max()));
        
        //TODO
        // static_assert(e8 < 1E-4);
    }

    {
        [[maybe_unused]] constexpr float f1 = _IQNtoF<1>(0);
        [[maybe_unused]] constexpr float f2 = _IQNtoF<1>(1);
        [[maybe_unused]] constexpr float f3 = _IQNtoF<1>(90);
        [[maybe_unused]] constexpr float f4 = _IQNtoF<1>(900);
        [[maybe_unused]] constexpr float f5 = _IQNtoF<1>(9000);
        [[maybe_unused]] constexpr float f6 = _IQNtoF<1>(900000);
        [[maybe_unused]] constexpr float f7 = _IQNtoF<1>(int32_t(9E7));
        [[maybe_unused]] constexpr float f8 = _IQNtoF<1>(std::numeric_limits<int32_t>::max());

        //TODO
        // constexpr float e8 = std::abs(f8 - (std::numeric_limits<int32_t>::max()/2));
        // static_assert(e8 < 1E-4);
    }

    {
        // 最大正值边界
        // 检查最大正数的转换
        static_assert(std::abs(_IQNtoF<16>(std::numeric_limits<int32_t>::max()) - (static_cast<float>(std::numeric_limits<int32_t>::max()) / 65536.0f)) < 1,
                    "INT_MAX转换测试");

        // constexpr auto max_value = _IQNtoF<16>(std::numeric_limits<int32_t>::max() / 2);
        [[maybe_unused]] constexpr auto max_value = _IQNtoF<16>(0x40000000);
        [[maybe_unused]] constexpr auto max_value2 = _IQNtoF<16>(std::numeric_limits<int32_t>::max());

        // 验证Q值范围
        // Q可能为0-31，但代码中的计算可能有问题
        // 特别是当Q=0或Q=31时的边界情况
        static_assert(_IQNtoF<0>(1) == 1.0f, "Q=0转换测试");
    }



    {
        // 测试舍入溢出
        // 找一個会使 uiq31Input + 0x0080 溢出的值
        // 需要 uiq31Input >= 0xFFFFFF80
        constexpr int32_t round_overflow_test = 0x7FFFFFFF;  // 在Q格式中
        // 当Q=0时，这对应很大的浮点数
        [[maybe_unused]] constexpr auto i = int64_t(_IQNtoF<0>(round_overflow_test));
        // static_assert( == round_overflow_test);
    }

    {
        constexpr int32_t large_for_q = 0x40000000;  // Q=1时对应0.5
        static_assert(_IQNtoF<31>(large_for_q) == 0.5f, "大值指数调整测试");
    }

    {
        // 精度损失测试
        // 测试转换的对称性：f -> iq -> f 应该保持精度
        constexpr float original = 0.123456f;
        constexpr int32_t iq_val = _IQFtoN<16>(original);  // 假设有正向转换函数
        constexpr float recovered = _IQNtoF<16>(iq_val);
        static_assert(std::abs(original - recovered) < 3E-5, "往返转换精度测试");
    }

    {
        // 零值符号测试
        // 确保正零和负零都返回0.0f
        static_assert(_IQNtoF<16>(0) == 0.0f, "零值转换");
        static_assert(_IQNtoF<16>(-0) == 0.0f, "负零转换");
    }

    // 测试Q值极端情况
    // Q=31时，最小步长是 1/2^31 ≈ 4.6566e-10
    static_assert(_IQNtoF<31>(1) == 4.656612873077392578125e-10f, 
                "Q=31最小精度测试");

    //  验证指数构造的正确性
    // 检查指数部分的计算：0x3f80 + ((31 - Q) * ((uint32_t) 1 << (23 - 16)))
    // 0x3f80 = 127 << 7 (IEEE指数偏移127，但这里用16位存储)
    // 当Q=16时，指数部分应该是127 << 7 + (15 << 7) = 142 << 7
    constexpr float test_q16 = _IQNtoF<16>(0x8000);  // 应该得到0.5
    static_assert(test_q16 == 0.5f, "Q=16的0.5测试");



    {
        constexpr float f0 = _IQNtoF<16>(0x8000);
        constexpr float f1 = _IQNtoF<16>(0x4000);
        constexpr float f2 = _IQNtoF<16>(0xC000);

        [[maybe_unused]] constexpr float f3 = _IQNtoF<16>(0x3FFFFFFF);
        [[maybe_unused]] constexpr float f4 = _IQNtoF<16>(0x40000000);
        [[maybe_unused]] constexpr float f5 = _IQNtoF<16>(0x40000101);
        [[maybe_unused]] constexpr int32_t i5 = int32_t(_IQNtoF<16>(0x40000010) * (1u << 16));
        [[maybe_unused]] constexpr float f6 = _IQNtoF<16>(0x7fffffff);

        auto cmp_inc = [](const int32_t a, const int32_t b) -> bool{
            if(float(a) < float(b)){
                return _IQNtoF<16>(a) < _IQNtoF<16>(b);
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
    static_assert(_IQNtoF<16>(0) == 0.0f, "零值测试");
    static_assert(_IQNtoF<16>(std::numeric_limits<int32_t>::min()) < 0.0f,
                "INT_MIN测试");
}

[[maybe_unused]] void test_f_to_n(){

    // 添加更多测试
    static_assert(_IQFtoN<16>(0.0f) == 0);
    static_assert(_IQFtoN<16>(-0.0f) == 0);
    static_assert(_IQFtoN<16>(1.0f) == 65536);  // Q16: 1.0 = 65536
    static_assert(_IQFtoN<16>(-1.0f) == -65536);

    // 测试非常小的数
    static_assert(_IQFtoN<16>(0.0001f) == 6);  // 近似值
    static_assert(_IQFtoN<16>(-0.0001f) == -6);

    // 测试Q不同值的情况
    static_assert(_IQFtoN<8>(1.0f) == 256);    // Q8: 1.0 = 256
    static_assert(_IQFtoN<8>(0.5f) == 128);
    static_assert(_IQFtoN<24>(1.0f) == 16777216);  // Q24: 1.0 = 16777216

    static_assert(_IQFtoN<16>(-32768.0f / 65536.0f) == -32768);

    static_assert(_IQFtoN<16>(6556.0f) == 6556 << 16);
    static_assert(_IQFtoN<16>(.04f) == int32_t(0.04 * 65536));
    static_assert(_IQFtoN<16>(.001f) == int32_t(0.001 * 65536));

    static_assert(_IQFtoN<16>(-6556.0f) == -6556 * 65536);
    static_assert(_IQFtoN<16>(-0.04f) == int32_t(-0.04 * 65536));
    static_assert(_IQFtoN<16>(-0.001f) == int32_t(-0.001 * 65536));

    static_assert(_IQFtoN<24>(66.0f) == 66 << 24);
    static_assert(_IQFtoN<24>(.04f) == int32_t(0.04 * (1 << 24)));
    static_assert(_IQFtoN<24>(.0001f) == int32_t(0.0001 * (1 << 24)));

    static_assert(_IQFtoN<24>(-66.0f) == -66 * (1 << 24));
    static_assert(_IQFtoN<24>(-0.04f) == int32_t(-0.04 * (1 << 24)));
    static_assert(_IQFtoN<24>(-0.0001f) == int32_t(-0.0001 * (1 << 24)));

    static_assert(_IQFtoN<16>(0.0f) == 0);
    static_assert(_IQFtoN<16>(-0.0f) == 0);
    static_assert(_IQFtoN<16>(1.0f) == 65536);  // Q16: 1.0 = 65536
    static_assert(_IQFtoN<16>(-1.0f) == -65536);
    static_assert(_IQFtoN<16>(0.5f) == 32768);
    static_assert(_IQFtoN<16>(-0.5f) == -32768);
    static_assert(_IQFtoN<16>(0.25f) == 16384);
    static_assert(_IQFtoN<16>(1.5f) == 98304);  // 1.5 * 65536 = 98304

    // 测试非常小的数
    static_assert(_IQFtoN<16>(0.0001f) == 6);  // 近似值
    static_assert(_IQFtoN<16>(-0.0001f) == -6);

    // 测试Q不同值的情况
    static_assert(_IQFtoN<8>(1.0f) == 256);    // Q8: 1.0 = 256
    static_assert(_IQFtoN<8>(0.5f) == 128);
    static_assert(_IQFtoN<24>(1.0f) == 16777216);  // Q24: 1.0 = 16777216

    static_assert(_IQFtoN<16>(-32768.0f / 65536.0f) == -32768);



    static_assert(_IQFtoN<16>(0) == 0);
    static_assert(_IQFtoN<16>(1E-9) == 0);

    // 符号处理逻辑错误：-2.0在Q=1格式下应返回INT_MIN，但代码可能错误处理
    static_assert(_IQFtoN<1>(-2.0f) == std::numeric_limits<int32_t>::min(), "符号边界处理失败");

    // 溢出检测不充分：2.5在Q=30格式下应溢出到INT_MAX，但移位计算可能错误
    static_assert(_IQFtoN<30>(2.5f) == std::numeric_limits<int32_t>::max(), "正溢出检测失败");


    // 极小数处理：最小正非规格化数应返回0
    static_assert(_IQFtoN<16>(std::numeric_limits<float>::denorm_min()) == 0, "非规格化数处理失败");

    // 零值处理：正负零都应返回0
    static_assert(_IQFtoN<8>(0.0f) == 0, "正零处理失败");
    static_assert(_IQFtoN<8>(-0.0f) == 0, "负零处理失败");

    // 无穷大处理
    static_assert(_IQFtoN<16>(std::numeric_limits<float>::infinity()) == std::numeric_limits<int32_t>::max(), "正无穷处理失败");
    static_assert(_IQFtoN<16>(-std::numeric_limits<float>::infinity()) == std::numeric_limits<int32_t>::min(), "负无穷处理失败");

    // NaN处理
    static_assert(_IQFtoN<16>(std::numeric_limits<float>::quiet_NaN()) == 0, "NaN处理失败");

    // 精确值测试：1.0在Q=15格式下应为32768
    static_assert(_IQFtoN<15>(1.0f) == 32768, "精确值转换失败");

    // 负精确值测试：-0.5在Q=15格式下应为-16384
    static_assert(_IQFtoN<15>(-0.5f) == -16384, "负精确值转换失败");

    // 过小值处理：极小的负数应返回0或INT_MIN
    static_assert(_IQFtoN<16>(-1E-20f) == 0 || _IQFtoN<16>(-1E-20f) == std::numeric_limits<int32_t>::min(), "极小负数处理失败");


    // 利用符号处理逻辑错误
    // 原始代码中：if (result == std::numeric_limits<int32_t>::min()) return min;
    // 但result是正数，永远不会等于INT_MIN，所以负数边界处理完全错误
    static_assert(_IQFtoN<30>(-1.9999999f) != std::numeric_limits<int32_t>::min(), 
                "符号处理逻辑错误应导致错误结果");

    // 溢出检测的边界条件
    // shift < -31 检查不够精确，应该考虑尾数的实际值
    // 例如 Q=0, f=1.0, exponent=0, shift=23-0-0=23 (右移)
    // 但如果是 Q=0, f=很大值，可能导致左移溢出
    constexpr float large_val = static_cast<float>((1ULL << 24) - 1);
    static_assert(_IQFtoN<0>(large_val) == std::numeric_limits<int32_t>::max(),
                "大值左移应检测溢出");


    // 左移溢出检测逻辑问题
    // 代码中的检查：if (mantissa_bits > (UINT32_MAX >> left_shift))
    // 但UINT32_MAX >> left_shift 可能为0，导致错误检测
    // 例如：left_shift=32时，UINT32_MAX>>32=0，任何mantissa_bits>0都会通过检测
    static_assert(_IQFtoN<31>(2.0f) != 0, "左移32位处理可能有问题");

    // 非规格化数的指数计算
    // 原始代码对非规格化数直接返回0，但IEEE754非规格化数有特殊指数值
    constexpr float denorm = std::bit_cast<float>(0x00000001U); // 最小正非规格化数
    static_assert(_IQFtoN<0>(denorm) == 0, "非规格化数应返回0");

    // 检查符号位与取负的交互
    // 当is_negative=true且unsigned_result=0时，应该返回0而不是-0
    static_assert(_IQFtoN<16>(-0.0000001f) == 0, "极小负数应返回0而非-0");


    static_assert(_IQFtoN<30>(1E-9) == int32_t((1u << 30) * (1E-9)), "极小正数应正确处理");
    static_assert(_IQFtoN<30>(1E-13) == int32_t((1u << 30) * (1E-13)), "极小正数应正确处理");
    static_assert(_IQFtoN<30>(1E-20) == int32_t((1u << 30) * (1E-20)), "极小正数应正确处理");

    // 验证负零的符号处理
    // 负零在IEEE754中符号位为1，但值是0
    // 代码中is_negative=true，但最后会取负，得到-0，但转换为int32_t应该是0
    static_assert(_IQFtoN<8>(0.0f) == _IQFtoN<8>(-0.0f), "正负零应得到相同结果");
}

}