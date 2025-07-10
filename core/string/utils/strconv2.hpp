#pragma once

#include <cstdint>
#include <concepts>
#include <tuple>
#include <string_view>

#include "prelude.hpp"

#include "core/utils/Errno.hpp"
#include "core/math/iq/iq_t.hpp"
#include "core/string/string_ref.hpp"
#include "core/utils/Result.hpp"
#include "core/magic/enum_traits.hpp"


namespace ymd::strconv2 {



void reverse_str(StringRef str){
	auto len = str.length();

	if(unlikely(len == 0)) return;

	len -= 1;
	for(size_t i = 0; i < len / 2 + (len % 2); i++){
		std::swap(str[i],str[len - i]);
	}
}

struct FstrDump final{
    int32_t digit_part;
	int32_t frac_part;
	int32_t scale;

	static constexpr DestringResult<FstrDump> from_str(const std::string_view str) {
		if (str.empty()) {
			return Err(DestringError::EmptyString);
		}

		int32_t digit_part = 0;
		int32_t frac_part = 0;
		int32_t scale = 1;
		bool passed_point = false;
		bool is_minus = false;
		size_t index = 0;
		const auto len = str.size();

		// 处理符号
		if (str[index] == '+' || str[index] == '-') {
			is_minus = (str[index] == '-');
			++index;
			if (index == len) {
				return Err(DestringError::NoDigits);  // 只有符号没有数字
			}
		}

		// 主解析循环
		bool has_digits = false;
		while (index < len) {
			const char chr = str[index];
			
			if (chr == '.') {
				if (passed_point) {
					return Err(DestringError::InvalidPoint);  // 多个小数点
				}
				passed_point = true;
				++index;
				continue;
			}

			if (!std::isdigit(static_cast<unsigned char>(chr))) {
				return Err(DestringError::InvalidNumber);  // 非法字符
			}

			has_digits = true;
			const int digit = chr - '0';

			if (!passed_point) {
				// 检查整数部分溢出
				if (digit_part > (std::numeric_limits<int32_t>::max() - digit) / 10) {
					return Err(DestringError::DigitOverflow);
				}
				digit_part = digit_part * 10 + digit;
			} else {
				// 检查小数部分溢出和精度限制
				if (scale > std::numeric_limits<int32_t>::max() / 10) {
					return Err(DestringError::FracOverflow);
				}
				frac_part = frac_part * 10 + digit;
				scale *= 10;
			}

			++index;
		}

		if (!has_digits) {
			return Err(DestringError::NoDigits);  // 没有有效数字
		}

		if (is_minus) {
			digit_part = -digit_part;
			frac_part = -frac_part;
		}

		return Ok(FstrDump{
			.digit_part = digit_part,
			.frac_part = frac_part,
			.scale = scale
		});
	}
};

__fast_inline static constexpr bool is_digit(const char chr){return chr >= '0' && chr <= '9';}
__fast_inline static constexpr bool is_alpha(const char chr) {
    return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z');
}

__fast_inline static constexpr char digit2char(uint8_t digit) noexcept {
	return digit + ((digit > 9) ? ('A' - 10) : '0');
}


static constexpr bool is_numeric(const std::string_view str) {
	const auto len = str.length();
	bool has_digit = false;
	bool has_dot = false;
	bool has_sign = false;

	for (size_t i = 0; i < len; i++) {
		char chr = str[i];
		if(chr == '\0'){
			break;
		} else if (is_digit(chr)) {
			has_digit = true;
		} else if (chr == '.') {
			if (has_dot || !has_digit) {
				return false;
			}
			has_dot = true;
		} else if (chr == '+' || chr == '-') {
			if (has_sign || has_digit || has_dot) {
				return false;
			}
			has_sign = true;
		} else {
			return false;
		}
	}
	return has_digit;
}

static constexpr bool is_digit(const std::string_view str){
	const auto len = str.length();
    for(size_t i = 0; i < len; i++){
		char chr = str[i];
        if(!is_digit(chr)) return false;
		if(chr == '\0') break;
    }
    return true;
}

namespace details{


static constexpr inline uint32_t exp10_map[] = {
    1UL, 
    10UL, 
    100UL, 
    1000UL, 

    10000UL, 
    100000UL, 
};


__fast_inline constexpr size_t fast_log10(uint64_t value, const Radix radix_e){
    if(value == 0) return 1;

    size_t i = 0;
    uint64_t sum = 1;
	const auto radix = radix_e.count();
    while(value >= sum){
        sum *= radix;
        i++;
    }
    return MAX(i, 1);
}

__fast_inline constexpr uint32_t fast_exp10(const size_t n){
	return exp10_map[n];
}

template<integral T>
struct IntFromStringHelper{
	static constexpr DestringResult<int> impl(const std::string_view str) {
		const auto len = str.length();
		uint32_t ret = 0;
		bool is_negtive = false;

		for(size_t i = 0; i < len; i++){
			char chr = str[i];

			switch(chr){
				case '\0':
					break;
				case '-':
					if(is_negtive) break;
					is_negtive = true;
					break;
				case '0' ... '9':
					ret *= 10;
					ret += chr - '0';
					break;
			}
		}

		if(ret < 0) ret = INT32_MAX;
		if(is_negtive) ret = - ret;
		return Ok(int(ret));
	}
};

template<size_t Q>
struct IqFromStringHelper{
	static constexpr DestringResult<iq_t<Q>> impl(const std::string_view str){
		const auto dump = ({
			const auto res = FstrDump::from_str(str);
			if(res.is_err()) return Err(res.unwrap_err());
			res.unwrap();
		});
		
		const iq_t<Q> ret = iq_t<Q>(dump.digit_part) 
			+ iq_t<Q>::from_i32((dump.frac_part << Q) / dump.scale);

		return Ok(ret);
	}
};

template<integral T>
struct IntToStringHelper{
	static constexpr TostringResult<StringRef> impl(T value, StringRef str, Radix radix_e){

		const auto radix = radix_e.count();
		const bool is_negative = value < 0;

		// Handle zero explicitly
		if (unlikely(value == 0)) {
			if (str.length() < 1U) return Err(TostringError::OutOfMemory);
			str[0] = '0';
			return Ok(str.substr_unchecked(1));
		}

		// Make value positive
		using U = std::make_unsigned_t<T>;
		U uvalue = is_negative ? (U)(-(value + 1)) + 1 : (U)value;

		// Compute length safely
		size_t len = fast_log10(uvalue, radix_e) + (is_negative ? 1 : 0);
		if (str.length() < len) return Err(TostringError::OutOfMemory);

		int i = len - 1;

		do {
			uint8_t digit = uvalue % radix;
			str[i--] = digit2char(digit);
			uvalue /= radix;
		} while (uvalue > 0 && i >= 0);

		if (is_negative) {
			str[0] = '-';
		}

		return Ok(str.substr_unchecked(len));
	}


};

struct UIntToFixedStringHelper {
    const size_t len;
	constexpr TostringResult<StringRef> impl(uint32_t value, StringRef str, Radix radix_e) const {
		if (str.length() < len) {
			return Err(TostringError::OutOfMemory);
		}

		const auto radix = radix_e.count();

		size_t i = len;
		do {
			uint8_t digit = value % radix;
			str[--i] = digit2char(digit);
			value /= radix;
		} while (i > 0 && value > 0);

		while (i > 0) {
			str[--i] = '0';
		}

		return Ok(str.substr_unchecked(len));
	}
};

struct Iq16ToStringHelper{
	//TODO eps为5时计算会溢出 暂时限制eps=5的情况
	static constexpr TostringResult<StringRef> impl(iq_t<16> value_, StringRef str, const Eps eps_){
		if(str.length() == 0) return Err(TostringError::OutOfMemory);
		//TODO 支持除了Q16格式外其他格式转换到字符串的函数 
		constexpr size_t Q = 16;

		const auto value = std::bit_cast<int32_t>(value_.qvalue());
		const auto eps = MIN(eps_.count(), 4);

		const bool is_negtive = value < 0;
		const uint32_t abs_value = ABS(value);
		constexpr uint32_t lower_mask = (Q == 31) ? 0x7fffffffu : uint32_t(((1 << Q) - 1));

		const uint32_t frac_part = uint32_t(abs_value) & lower_mask;

		const uint32_t scale = fast_exp10(eps);

		const uint32_t fs = frac_part * scale;
		
		const bool upper_round = (fs & lower_mask) >= (lower_mask >> 1);

		const uint32_t frac_int = (fs >> Q) + upper_round;
		const uint32_t digit_part = (uint32_t(abs_value) >> Q) + bool(frac_int >= scale);

		if(is_negtive){
			if(str.length() == 0) 
				return Err(TostringError::OutOfMemory);

			str[0] = '-';
			str = str.substr_unchecked(1);
		}

		str = ({
			const auto res = IntToStringHelper<int>::impl(digit_part, str, Radix::Dec);
			if(res.is_err()) return Err(res.unwrap_err());
			res.unwrap();
		});

		if(str.length() < size_t(1 + eps))
			return Err(TostringError::OutOfMemory);

		str[0] = '.';
		str = str.substr_unchecked(1);

		return UIntToFixedStringHelper{eps}.impl(
			frac_int, str, Radix::Dec);
		
		return Ok(str);
	}
};

}

template<integral T>
static constexpr TostringResult<StringRef> to_str(
	T value, 
	StringRef str, 
	Radix radix = Radix(Radix::Kind::Dec)
){
	return details::IntToStringHelper<T>::impl(value, str, radix);
}

template<size_t Q>
static constexpr TostringResult<StringRef> to_str(iq_t<Q> value, StringRef str, const Eps eps = Eps(3)){
	return details::Iq16ToStringHelper::impl(value, str, eps);
}

template<size_t Q>
static constexpr DestringResult<iq_t<Q>> iq_from_str(std::string_view str){
	return details::IqFromStringHelper<Q>::impl(str);
}

}