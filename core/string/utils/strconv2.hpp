#pragma once

#include <cstdint>
#include <concepts>
#include <tuple>
#include <string_view>

#include "prelude.hpp"

#include "core/utils/Errno.hpp"
#include "core/math/iq/iq_t.hpp"
#include "core/string/string_ref.hpp"
#include "core/string/string_view.hpp"
#include "core/utils/Result.hpp"
#include "core/magic/enum_traits.hpp"
#include "core/stream/ostream.hpp"

namespace ymd::strconv2 {


struct FstrDump final{
    int32_t digit_part;
	int32_t frac_part;
	uint32_t scale;

	static constexpr DestringResult<FstrDump> from_str(const StringView str) {
		if (str.length() == 0) {	
			return Err(DestringError::EmptyString);
		}

		int32_t digit_part = 0;
		int32_t frac_part = 0;
		uint32_t scale = 1;

		struct Status{
			uint8_t passed_dot:1 = false;
			uint8_t has_pre_dot_digits:1 = false;
			uint8_t has_post_dot_digits:1 = false;
			uint8_t is_negative:1 = false;
		};

		Status status;
		const auto len = str.size();
		size_t index = 0;

		// 略过空格
		while (index < len && str[index] == ' ') {
			index++;
		}

		// 处理符号
		if (str[index] == '+' || str[index] == '-') {
			status.is_negative = (str[0] == '-');
			++ index;
			if (len == 1) {
				return Err(DestringError::OnlySignFounded);  // 只有符号没有数字
			}
		}

		while (index < len) {
			const char chr = str[index];
			
			switch (chr) {
				case '0' ... '9':{  // Digit case (GCC/Clang extension)

					const uint8_t digit = chr - '0';
					
					constexpr int32_t MAX_INT_NUM = (std::numeric_limits<int32_t>::max() - 9) / 10;
					if (!status.passed_dot) {
						status.has_pre_dot_digits = true;
						// Check integer part overflow
						if (digit_part > MAX_INT_NUM) {
							return Err(DestringError::DigitOverflow);
						}
						digit_part = digit_part * 10 + digit;
					} else {
						status.has_post_dot_digits = true;
						// Check fractional part overflow
						if (scale > MAX_INT_NUM) {
							return Err(DestringError::FracOverflow);
						}
						frac_part = frac_part * 10 + digit;
						scale *= 10;
					}
					++index;
					break;
				}

				case '+':
					return Err(DestringError::UnexpectedPositive);
				case '-':
					return Err(DestringError::UnexpectedNegative);
				case '.':  // Handle decimal dot
					if (status.passed_dot) {
						return Err(DestringError::MultipleDot);  // Multiple decimal dots
					}
					status.passed_dot = true;
					++index;
					break;
				case 'a' ... 'z':
					return Err(DestringError::UnexpectedAlpha);
				case 'A' ... 'Z':
					return Err(DestringError::UnexpectedAlpha);
				case ' ':
					return Err(DestringError::UnexpectedSpace);
				default:  // Invalid characters
					return Err(DestringError::UnexpectedChar);
			}
		}

		if(status.passed_dot){//有小数的情况
			if((!status.has_post_dot_digits))
				return Err(DestringError::NoDigitsAfterDot);
		}else{//只有整数的情况
			if (!status.has_pre_dot_digits) {
				return Err(DestringError::NoDigitsAfterSign);  // 没有有效数字
			}
		}

		if(status.is_negative){
			digit_part = -digit_part;
			frac_part = -frac_part;
		}

		return Ok(FstrDump{
			.digit_part = digit_part,
			.frac_part = frac_part,
			.scale = (status.has_post_dot_digits ? scale : 0)
		});
	}

	friend OutputStream & operator<<(OutputStream & os, const FstrDump & dump) {
		os << dump.digit_part << os.splitter() << dump.frac_part << os.splitter() << dump.scale;
		return os;
	}
};

__fast_inline static constexpr bool is_digit(const char chr){
		return chr >= '0' && chr <= '9';}
__fast_inline static constexpr bool is_alpha(const char chr) {
    return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z');
}

__fast_inline static constexpr char digit2char(uint8_t digit) noexcept {
	return digit + ((digit > 9) ? ('A' - 10) : '0');
}


static constexpr bool is_numeric(const StringView str) {
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

static constexpr bool is_digit(const StringView str){
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

template<typename T>
__fast_inline constexpr T div_5(T x){return (((int64_t)x*0x66666667L) >> 33);}

template<typename T>
__fast_inline constexpr T div_10(T x){(((int64_t)x*0x66666667L) >> 34);}

template<typename T>
requires(std::is_unsigned_v<T>)
__fast_inline constexpr size_t fast_uint_str_len(T value, const Radix radix_e) {
    if (value == 0) return 1;

    const auto radix = radix_e.count();

    // 优化 1: radix 是 2 的幂时，用位运算
    if ((radix & (radix - 1)) == 0) {
        const int log2_radix = __builtin_ctz(radix); // log2(radix)
        const int leading_zeros = __builtin_clz(value | 1); // 63 - bit_width(value)
        const int bit_width = 64 - leading_zeros;
        return (bit_width + log2_radix - 1) / log2_radix;
    }

    // 优化 2: 通用情况，用对数近似计算
    size_t length = 1;
    T sum = radix;
    while (value >= sum) {
        sum *= radix;
        length++;
    }
    return length;
}

__fast_inline constexpr uint32_t fast_exp10(const size_t n){
	return exp10_map[n];
}

template<integral T>
struct IntFromStringHelper{
	static constexpr DestringResult<int> conv(const StringView str) {
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
	static constexpr DestringResult<iq_t<Q>> conv(const StringView str){
		const auto dump = ({
			const auto res = FstrDump::from_str(str);
			if(res.is_err()) return Err(res.unwrap_err());
			res.unwrap();
		});
		
		const iq_t<Q> ret = 
			iq_t<Q>(dump.digit_part) 
			+ (dump.scale ? 
				iq_t<Q>::from_i32((int32_t(dump.frac_part) * int32_t(1 << Q)) / int32_t(dump.scale))
				: 0)
			;

		return Ok(ret);
	}
};


template<size_t N>
__fast_inline static constexpr std::tuple<uint32_t, uint8_t> fast_div(uint32_t x){
	if constexpr (std::has_single_bit(N)){
		constexpr size_t shift = __builtin_ctz(N);
		constexpr size_t mask = N - 1;
		return {x >> shift, uint8_t(x & mask)};
	}else{
		const auto q = iq_t<16>(1.0 / N);
		const auto r = uint32_t(x * q);
		return {r, uint8_t(x - r * N)};
	}
}

enum class Int2StringRole:uint8_t{
	UnFixed,
	Fixed
};

template<size_t N, Int2StringRole R>
__fast_inline static constexpr void fast_2str(uint32_t value, char * str, size_t i){
	do {
		const auto [q, r] = fast_div<N>(value);
		str[--i] = digit2char(r);
		value = q;
	} while (i > 0 && value >= 0);

	if constexpr(R == Int2StringRole::Fixed){
		while (i > 0) {
			str[--i] = '0';
		}
	}

}

template<Int2StringRole R>
__fast_inline static constexpr void dyn_2str(uint32_t value, char * str, size_t i, uint8_t radix){
	switch(radix){
		case 2: fast_2str<2, R>(value, str, i); break;
		case 8: fast_2str<2, R>(value, str, i); break;
		// case 10: fast_2str<10, R>(value, str, i); break;
		case 16: fast_2str<16, R>(value, str, i); break;
		
		default:{
			do {
				uint8_t digit = value % radix;
				str[--i] = digit2char(digit);
				value /= radix;
			} while (i > 0 && value >= 0);

			if constexpr( R == Int2StringRole::Fixed){
				while (i > 0) {
					str[--i] = '0';
				}
			}
			break;
		}

	}
}

template<integral T>
struct IntToStringHelper{
	static constexpr TostringResult<StringRef> conv(T value, StringRef str, Radix radix_e){
		// Handle zero explicitly
		if (unlikely(value == 0)) {
			if (str.length() < 1U) return Err(TostringError::OutOfMemory);
			str[0] = '0';
			return Ok(str.substr_unchecked(1));
		}

		const auto radix = radix_e.count();
		const bool is_negative = value < 0;

		// Make value positive
		using U = std::make_unsigned_t<T>;
		U uvalue = is_negative ? (U)(-(value + 1)) + 1 : (U)value;

		// Compute length safely
		size_t len = fast_uint_str_len(uvalue, radix_e) + (is_negative ? 1 : 0);
		if (str.length() < len) return Err(TostringError::OutOfMemory);

		dyn_2str<Int2StringRole::UnFixed>(uvalue, str.data(), len, radix);

		if (is_negative) {
			str[0] = '-';
		}

		return Ok(str.substr_unchecked(len));
	}


};

struct UIntToFixedStringHelper{
    const size_t len;

	constexpr TostringResult<StringRef> conv(uint32_t value, StringRef str, Radix radix_e) const {
		if (str.length() < len) {
			return Err(TostringError::OutOfMemory);
		}

		const auto radix = radix_e.count();
	
		dyn_2str<Int2StringRole::Fixed>(value, str.data(), len, radix);

		return Ok(str.substr_unchecked(len));
	}

private:

};

struct Iq16ToStringHelper{
	//TODO eps为5时计算会溢出 暂时限制eps=5的情况
	static constexpr TostringResult<StringRef> conv(iq_t<16> value_, StringRef str, const Eps eps_){
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
			const auto res = IntToStringHelper<int>::conv(digit_part, str, Radix::Dec);
			if(res.is_err()) return Err(res.unwrap_err());
			res.unwrap();
		});

		if(str.length() < size_t(1 + eps))
			return Err(TostringError::OutOfMemory);

		str[0] = '.';
		str = str.substr_unchecked(1);

		return UIntToFixedStringHelper{eps}.conv(
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
	return details::IntToStringHelper<T>::conv(value, str, radix);
}

template<size_t Q>
static constexpr TostringResult<StringRef> to_str(iq_t<Q> value, StringRef str, const Eps eps = Eps(3)){
	return details::Iq16ToStringHelper::conv(value, str, eps);
}

template<size_t Q>
static constexpr DestringResult<iq_t<Q>> str_to_iq(StringView str){
	return details::IqFromStringHelper<Q>::conv(str);
}


template<typename T>
static constexpr DestringResult<T> from_str(StringView str){
	if constexpr (std::is_same_v<StringView, T>)
		return Ok(str);
	if constexpr (is_fixed_point_v<T>)
		return str_to_iq<T::q_num>(str);
	else if constexpr(std::is_integral_v<T>)
		return details::IntFromStringHelper<T>::conv(str);
}

}