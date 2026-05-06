#pragma once

#include <cstdint>
#include "fp32.hpp"

#include "core/math/real.hpp"

namespace ymd::math{
struct alignas(1) [[nodiscad]] fp8_e4m3 final{
	uint8_t frac:3;
	uint8_t exp:4;
	uint8_t sign:1;

	template<size_t Q, typename D>
	constexpr fp8_e4m3(fixed<Q, D> qv):
		fp8_e4m3(float(qv)){;}
	constexpr fp8_e4m3(const fp8_e4m3 & other) = default;
	constexpr fp8_e4m3(int iv){
		if(iv < 0){
			sign = true;
			iv = -iv;
		}
		exp = (iv >> 4) - 1;
		frac = (iv & 0xF) << 1;
	}
	
	constexpr fp8_e4m3(float fv){

		fp32 conversion(fv);
		
		sign = conversion.sign;
		exp = (conversion.exp - (127 - 15));
		frac = (conversion.frac >> (23 - 10)) & 0x7; // 只保留最低三位有效位
		
		// 处理特殊值
		if (exp == 0 && conversion.frac == 0) { // 零
			exp = 0;
			frac = 0;
		} else if (exp == 255) { // 无穷大或 NaN
			exp = 15;
			frac = 0;
		} else if (exp > 15) { // 溢出
			exp = 15;
			frac = 0;
		}
	}
	
	constexpr fp8_e4m3(double dv):fp8_e4m3(float(dv)){}
	
	explicit constexpr operator int() const{
		return 0;
	}

	explicit constexpr operator float() const {
		fp32 conversion;
		
		conversion.sign = sign;
		conversion.exp = exp + (127 - 15); // 调整指数偏移量
		conversion.frac = (frac << (23 - 10)); // 左移以填充更高位的0
		
		// 浮点数的隐含位
		conversion.frac |= (1 << 23);
		
		return static_cast<float>(conversion);
	}

	template<size_t Q, typename D>
	explicit constexpr operator fixed<Q, D>() const {
		return fixed(float(*this));
	}
};

struct alignas(1) [[nodiscard]] fp8_e5m2 final{
	uint8_t frac:2;
	uint8_t exp:5;
	uint8_t sign:1;
};


}

namespace std{
    template<>
    struct is_arithmetic<ymd::math::fp8_e4m3> : std::true_type {};
    template<>
    struct is_floating_point<ymd::math::fp8_e4m3> : std::true_type {};
    template<>
    struct is_signed<ymd::math::fp8_e4m3> : std::true_type {};
    template<>
    struct is_integral<ymd::math::fp8_e4m3> : std::false_type {};
    template<>
    struct is_scalar<ymd::math::fp8_e4m3> : std::true_type {};
    template<>
    struct is_trivially_copyable<ymd::math::fp8_e4m3> : std::true_type {};
    template<>
    struct is_trivially_destructible<ymd::math::fp8_e4m3> : std::true_type {};
    template<>
    struct is_trivially_default_constructible<ymd::math::fp8_e4m3> : std::true_type {};

    template<>
    struct numeric_limits<ymd::math::fp8_e4m3> {
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
        static constexpr int digits = 4;      // 1 + 3 bits
        static constexpr int digits10 = 1;
        static constexpr int max_digits10 = 2;
        static constexpr int radix = 2;
        static constexpr int min_exponent = -7;
        static constexpr int min_exponent10 = -2;
        static constexpr int max_exponent = 8;
        static constexpr int max_exponent10 = 2;

        static constexpr ymd::math::fp8_e4m3 min() noexcept { return ymd::math::fp8_e4m3::from_bits(0x08); }
        static constexpr ymd::math::fp8_e4m3 lowest() noexcept { return ymd::math::fp8_e4m3::from_bits(0xFB); }
        static constexpr ymd::math::fp8_e4m3 max() noexcept { return ymd::math::fp8_e4m3::from_bits(0x7B); }
        static constexpr ymd::math::fp8_e4m3 epsilon() noexcept { return ymd::math::fp8_e4m3::from_bits(0x30); }
        static constexpr ymd::math::fp8_e4m3 round_error() noexcept { return ymd::math::fp8_e4m3(0.5f); }
        static constexpr ymd::math::fp8_e4m3 infinity() noexcept { return ymd::math::fp8_e4m3::from_bits(0x78); }
        static constexpr ymd::math::fp8_e4m3 quiet_NaN() noexcept { return ymd::math::fp8_e4m3::from_bits(0x7C); }
        static constexpr ymd::math::fp8_e4m3 signaling_NaN() noexcept { return ymd::math::fp8_e4m3::from_bits(0x79); }
        static constexpr ymd::math::fp8_e4m3 denorm_min() noexcept { return ymd::math::fp8_e4m3::from_bits(0x01); }
    };

    template<>
    struct is_arithmetic<ymd::math::fp8_e5m2> : std::true_type {};
    template<>
    struct is_floating_point<ymd::math::fp8_e5m2> : std::true_type {};
    template<>
    struct is_signed<ymd::math::fp8_e5m2> : std::true_type {};
    template<>
    struct is_integral<ymd::math::fp8_e5m2> : std::false_type {};
    template<>
    struct is_scalar<ymd::math::fp8_e5m2> : std::true_type {};
    template<>
    struct is_trivially_copyable<ymd::math::fp8_e5m2> : std::true_type {};
    template<>
    struct is_trivially_destructible<ymd::math::fp8_e5m2> : std::true_type {};
    template<>
    struct is_trivially_default_constructible<ymd::math::fp8_e5m2> : std::true_type {};

    template<>
    struct numeric_limits<ymd::math::fp8_e5m2> {
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
        static constexpr int digits = 3;      // 1 + 2 bits
        static constexpr int digits10 = 1;
        static constexpr int max_digits10 = 2;
        static constexpr int radix = 2;
        static constexpr int min_exponent = -15;
        static constexpr int min_exponent10 = -4;
        static constexpr int max_exponent = 16;
        static constexpr int max_exponent10 = 4;

        static constexpr ymd::math::fp8_e5m2 min() noexcept { return ymd::math::fp8_e5m2::from_bits(0x04); }
        static constexpr ymd::math::fp8_e5m2 lowest() noexcept { return ymd::math::fp8_e5m2::from_bits(0xFB); }
        static constexpr ymd::math::fp8_e5m2 max() noexcept { return ymd::math::fp8_e5m2::from_bits(0x7B); }
        static constexpr ymd::math::fp8_e5m2 epsilon() noexcept { return ymd::math::fp8_e5m2::from_bits(0x30); }
        static constexpr ymd::math::fp8_e5m2 round_error() noexcept { return ymd::math::fp8_e5m2(0.5f); }
        static constexpr ymd::math::fp8_e5m2 infinity() noexcept { return ymd::math::fp8_e5m2::from_bits(0x7C); }
        static constexpr ymd::math::fp8_e5m2 quiet_NaN() noexcept { return ymd::math::fp8_e5m2::from_bits(0x7E); }
        static constexpr ymd::math::fp8_e5m2 signaling_NaN() noexcept { return ymd::math::fp8_e5m2::from_bits(0x7D); }
        static constexpr ymd::math::fp8_e5m2 denorm_min() noexcept { return ymd::math::fp8_e5m2::from_bits(0x01); }
    };

}