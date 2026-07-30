#pragma once

#include "core/math/fixed/fixed.hpp"
#include "fp16_utils.hpp"

namespace ymd::math{

// https://github.com/Matrixchung/iFOC
static constexpr bool HAS_NATIVE_FP16 = 
#if (defined(__arm__) || defined(__aarch64__)) && defined(__ARM_FP16_FORMAT_IEEE) && defined(__ARM_FEATURE_FP16_SCALAR_ARITHMETIC)
    true;
#else
    false;
#endif

struct alignas(2) [[nodiscard]] fp16 final{
    using Self = fp16;

    uint16_t mant:10;
    uint16_t exp:5;
    uint16_t sign:1;

    constexpr fp16() = default;

    template<size_t Q, typename D>
    constexpr fp16(fixed<Q, D> qv):fp16(float(qv)){;}
    constexpr fp16(const fp16& other) = default;

    [[nodiscard]] static constexpr fp16 from_bits(const uint16_t bits){
        return std::bit_cast<fp16>(bits);
    }

    [[nodiscard]] constexpr uint16_t to_bits() const noexcept {
        return std::bit_cast<uint16_t>(*this);
    }

    constexpr fp16(float f_val) {
        *this = f32_to_fp16(f_val);
    }

    constexpr fp16(int int_val){
        *this = int32_to_fp16(int_val);
    }
    constexpr fp16(const double val):fp16(static_cast<float>(val)){};

    [[nodiscard]] constexpr bool is_nan() const noexcept {
        return fp16_utils::fp16bits_is_nan(std::bit_cast<uint16_t>(*this));
    }

    [[nodiscard]] constexpr float to_f32() const {
        return fp16_utils::fp16bits_to_fp32_nonfpu(to_bits());
    }

    [[nodiscard]] explicit constexpr operator float() const noexcept {
        return to_f32();
    }

    template<typename D>
    requires (std::is_integral_v<D>)
    [[nodiscard]] explicit constexpr operator D() const noexcept {
        return static_cast<D>(static_cast<float>(*this));
    }


    template<size_t Q, typename D>
    [[nodiscard]] explicit constexpr operator fixed<Q, D>() const noexcept {
        return fixed<Q, D>(float(*this));
    }

    [[nodiscard]] explicit constexpr operator bool() const noexcept{
        return fp16_utils::fp16bits_is_zero(std::bit_cast<uint16_t>(*this));
    }
private:

    static constexpr fp16 int32_to_fp16_nonfpu(int32_t int_val) noexcept{
        return from_bits(fp16_utils::fp32_to_fp16bits_nonfpu(static_cast<float>(int_val)));
    }

    static constexpr fp16 int32_to_fp16(int32_t int_val){
        auto conv_with_fpu = [int_val]() -> fp16 {
            return from_bits(fp16_utils::fp32_to_fp16bits_nonfpu(static_cast<float>(int_val)));
        };

        if(std::is_constant_evaluated()){
            return conv_with_fpu();
        }

        #ifdef __FPU_PRESENT__
            return conv_with_fpu();
        #endif

        return int32_to_fp16_nonfpu(int_val);
    }

    static constexpr fp16 f32_to_fp16(const float f_val){
        return from_bits(fp16_utils::fp32_to_fp16bits_nonfpu(f_val));
    }
};

static_assert(sizeof(fp16) == 2);


constexpr bool isinf(const ymd::math::fp16 value) noexcept {
    return fp16_utils::fp16bits_is_inf(std::bit_cast<uint16_t>(value));
}

constexpr bool isnan(const ymd::math::fp16 value) noexcept {
    return fp16_utils::fp16bits_is_nan(std::bit_cast<uint16_t>(value));
}

}

namespace std{

    template<>
    struct is_arithmetic<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_floating_point<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_signed<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_integral<ymd::math::fp16> : std::false_type {};
    template<>
    struct is_scalar<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_trivially_copyable<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_trivially_destructible<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_trivially_default_constructible<ymd::math::fp16> : std::true_type {};

    template<>
    struct numeric_limits<ymd::math::fp16> {
        static constexpr bool is_specialized = true;
        static constexpr bool is_signed = true;
        static constexpr bool is_integer = false;
        static constexpr bool is_exact = false;
        static constexpr bool has_infinity = true;
        static constexpr bool has_quiet_NaN = true;
        static constexpr bool has_signaling_NaN = true;
        static constexpr std::float_denorm_style has_denorm = std::denorm_present;
        static constexpr bool has_denorm_loss = false;
        static constexpr std::float_round_style round_style = std::round_to_nearest;
        static constexpr bool is_iec559 = true;
        static constexpr bool is_bounded = true;
        static constexpr bool is_modulo = false;
        static constexpr int digits = 11;      // 1 + 10 bits
        static constexpr int digits10 = 3;
        static constexpr int max_digits10 = 5;
        static constexpr int radix = 2;
        static constexpr int min_exponent = -14;
        static constexpr int min_exponent10 = -4;
        static constexpr int max_exponent = 16;
        static constexpr int max_exponent10 = 4;

        static constexpr ymd::math::fp16 min() noexcept { return ymd::math::fp16::from_bits(0x0400); }
        static constexpr ymd::math::fp16 lowest() noexcept { return ymd::math::fp16::from_bits(0xFBFF); }
        static constexpr ymd::math::fp16 max() noexcept { return ymd::math::fp16::from_bits(0x7BFF); }
        static constexpr ymd::math::fp16 epsilon() noexcept { return ymd::math::fp16::from_bits(0x1400); }
        static constexpr ymd::math::fp16 round_error() noexcept { return ymd::math::fp16(0.5f); }
        static constexpr ymd::math::fp16 infinity() noexcept { return ymd::math::fp16::from_bits(0x7C00); }
        static constexpr ymd::math::fp16 quiet_NaN() noexcept { return ymd::math::fp16::from_bits(0x7E00); }
        static constexpr ymd::math::fp16 signaling_NaN() noexcept { return ymd::math::fp16::from_bits(0x7D00); }
        static constexpr ymd::math::fp16 denorm_min() noexcept { return ymd::math::fp16::from_bits(0x0001); }
    };

}
