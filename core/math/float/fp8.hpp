#pragma once

#include <cstdint>
#include "fp32.hpp"

#include "core/math/real.hpp"

namespace ymd::math{
struct [[nodiscad]] fp8_e4m3{
	union{
		uint8_t raw;
		struct{
			uint8_t frac:3;
			uint8_t exp:4;
			uint8_t sign:1;
		};
	};

	constexpr fp8_e4m3(fixed_t qv):fp8_e4m3(float(qv)){;}
	constexpr fp8_e4m3(const fp8_e4m3 & other):raw(other.raw){;}
	constexpr fp8_e4m3(int iv){
		sign = iv < 0;
		iv = abs(iv);
		exp = (iv >> 4) - 1;
		frac = (iv & 0xF) << 1;
	}
	
	constexpr fp8_e4m3(float fv){

		fp32 conversion;
		conversion.raw = *reinterpret_cast<uint32_t*>(&fv);
		
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
		
		// return *reinterpret_cast<const float*>(&conversion.raw);
		return std::bit_cast<float>(conversion.raw);
	}

	explicit constexpr operator fixed_t() const {
		return fixed_t(float(*this));
	}
};

struct [[nodiscard]] fp8_e5m2{
	union{
		uint8_t raw;
		struct{
			uint8_t frac:2;
			uint8_t exp:5;
			uint8_t sign:1;
		};
	};
}


#ifdef FP8_USE_E4M3
    using fp8 = fp8_e4m3;
#elif defined(FP8_USE_E5M2)
    using fp8 = fp8_e5m2;
#else
    #error "Please define FP8_USE_E4M3 or FP8_USE_E5M2"
#endif


}