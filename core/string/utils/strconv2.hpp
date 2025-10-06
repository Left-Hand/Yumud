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
#include "core/utils/angle.hpp"

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
		const auto len = str.length();
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

//是否为0~9的ascii字符
[[nodiscard]] __fast_inline static constexpr 
bool is_digit(const char chr){
		return chr >= '0' && chr <= '9';}

///是否为a~z, A~Z的ascii字符
[[nodiscard]] __fast_inline static constexpr 
bool is_alpha(const char chr) {
    return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z');
}

//数字转ascii字符
[[nodiscard]] __fast_inline static constexpr 
char digit2char(uint8_t digit) noexcept {
	return digit + ((digit > 9) ? ('A' - 10) : '0');
}

//是否为整数或小数
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


//是否为不含符号的整数
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


static constexpr inline uint32_t FAST_EXP10_TABLE[] = {
    1UL, 
    10UL, 
    100UL, 
    1000UL, 

    10000UL, 
    100000UL, 
};

template<typename T>
__fast_inline constexpr T fast_div_5(T x){return (((int64_t)x*0x66666667L) >> 33);}

template<typename T>
__fast_inline constexpr T fast_div_10(T x){(((int64_t)x*0x66666667L) >> 34);}

template<typename T>
requires(std::is_unsigned_v<T>)
__fast_inline constexpr size_t uint_to_num_chars(T value, const Radix radix) {
    if (value == 0) return 1;

    const auto radix_count = radix.count();

    // 优化 1: radix_count 是 2 的幂时，用位运算
    if ((radix_count & (radix_count - 1)) == 0) {
        const int log2_radix = __builtin_ctz(radix_count); // log2(radix_count)
        const int leading_zeros = __builtin_clz(value | 1); // 63 - bit_width(value)
        const int bit_width = 64 - leading_zeros;
        return (bit_width + log2_radix - 1) / log2_radix;
    }

    // 优化 2: 通用情况，用对数近似计算
    size_t length = 1;
    T sum = radix_count;
    while (value >= sum) {
        sum *= radix_count;
        length++;
    }
    return length;
}

__fast_inline constexpr uint32_t fast_exp10(const size_t n){
	return FAST_EXP10_TABLE[n];
}

template<integral T>
struct IntDeformatter{
	using U = std::make_unsigned_t<T>;
	static constexpr U MAX_BY_10 = static_cast<U>(std::numeric_limits<T>::max() / 10); 
	static constexpr DestringResult<T> defmt(const StringView str) {

		const auto len = str.length();
		if(len == 0) return Err(DestringError::EmptyString);

		U unsigned_ret = 0;
		char existing_sign = '\0';

		switch(str[0]){
			case '\0':
				return Err(DestringError::EmptyString);
			case '-':
				if constexpr(std::is_unsigned_v<T>)
					return Err(DestringError::NegForUnsigned);
				existing_sign = '-';
				break;
			case '+':
				existing_sign = '+';
				break;
		}

		for(size_t i = size_t(bool(existing_sign)); i < len; i++){
			char chr = str[i];

			switch(chr){
				case '\0':
					goto end_proc;
				case '-':
					if constexpr(std::is_unsigned_v<T>)
						return Err(DestringError::NegForUnsigned);
					if(existing_sign == '-') 
						return Err(DestringError::MultiplyNegative);
					return Err(DestringError::UnexpectedNegative);
				case '+':
					if(existing_sign == '+') 
						return Err(DestringError::MultiplyPositive);
					return Err(DestringError::UnexpectedPositive);
				case '0' ... '9':
					if(unlikely(unsigned_ret > MAX_BY_10))
						return Err(DestringError::Overflow);

					unsigned_ret = (10u * unsigned_ret) + static_cast<U>(chr - '0');
					break;
			}
		}
	end_proc:
		if(unlikely(unsigned_ret > std::numeric_limits<T>::max()))
			return Err(DestringError::Overflow);
		const auto ret_without_sign = static_cast<T>(unsigned_ret);
		return Ok(((existing_sign == '-') ? -ret_without_sign : ret_without_sign));
	}
};

template<>
struct IntDeformatter<bool>{
	static constexpr StringView TRUE_STR = StringView("true");
	static constexpr StringView FALSE_STR = StringView("false");
	static constexpr DestringResult<bool> defmt(const StringView str) {
		const auto len = str.length();
		switch(len){
			default:
				return Err(DestringError::InvalidBooleanLength);
			case 1:
				switch(str[0]){
					case '0':
						return Ok(false);
					case '1':
						return Ok(true);
					case '-':
						return Err(DestringError::UnexpectedNegative);
					case '+':
						return Err(DestringError::UnexpectedPositive);
					default:	
						return Err(DestringError::InvalidBooleanChar);
				}
			case 4:
				if(str != TRUE_STR)
					return Err(DestringError::InvalidBooleanAlpha);
				return Ok(true);
			case 5:
				if(str != FALSE_STR)
					return Err(DestringError::InvalidBooleanAlpha);
				return Ok(false);
		}
		__builtin_unreachable();
	}
};

template<typename IQ>
struct IqDeformatter{
	static constexpr size_t Q = IQ::q_num;
	static constexpr DestringResult<IQ> defmt(const StringView str){
		const auto dump = ({
			const auto res = FstrDump::from_str(str);
			if(res.is_err()) return Err(res.unwrap_err());
			res.unwrap();
		});
		
		const IQ frac_part = [&] -> IQ{
			if (dump.scale == 0) return 0; 
			return IQ::from_i32(
				(int32_t(dump.frac_part) * int32_t(1 << Q)) / int32_t(dump.scale)
			);
		}();

		return Ok(frac_part + IQ(dump.digit_part));
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

enum class PaddingStrategy:uint8_t{
	NoPadding,
	PaddingZero
};


//格式化u32到字符串(静态派发进制)
template<size_t Radix, PaddingStrategy S>
__fast_inline static constexpr void static_fmt_u32(
	StringRef str, 
	uint32_t value
){
	size_t i = str.length();
	do {
		const auto [q, r] = fast_div<Radix>(value);
		str[--i] = digit2char(r);
		value = q;
	} while (i > 0 && value >= 0);

	if constexpr(S == PaddingStrategy::PaddingZero){
		while (i > 0) {
			str[--i] = '0';
		}
	}
}

//格式化u32到字符串(动态派发进制)
template<PaddingStrategy S>
__fast_inline static constexpr void dyn_fmt_u32(
	StringRef str, 
	uint32_t value, 
	const uint8_t radix_count
){
	switch(radix_count){
		case 2: static_fmt_u32<2, S>(str, value); break;
		case 8: static_fmt_u32<8, S>(str, value); break;
		// case 10: static_fmt_u32<10, S>(str, value); break;
		case 16: static_fmt_u32<16, S>(str, value); break;
		
		default:{
			size_t i = str.length();
			do {
				uint8_t digit = value % radix_count;
				str[--i] = digit2char(digit);
				value /= radix_count;
			} while (i > 0 && value >= 0);

			if constexpr( S == PaddingStrategy::PaddingZero){
				while (i > 0) {
					str[--i] = '0';
				}
			}
			break;
		}

	}
}

template<integral T>
struct IntFormatter{
	[[nodiscard]] static constexpr 
	TostringResult<size_t> fmt(StringRef str, T value, Radix radix){
		if (str.length() == 0)
			return Err(TostringError::OutOfMemory);
			
		// Handle zero explicitly
		if (unlikely(value == 0)) {
			str[0] = '0';
			return Ok(1);
		}

		const auto radix_count = radix.count();
		const bool is_negative = value < 0;

		// Make value positive
		using U = std::make_unsigned_t<T>;
		U uvalue = is_negative ? (U)(-(value + 1)) + 1 : (U)value;

		// Compute length safely
		size_t len = uint_to_num_chars(uvalue, radix) + (is_negative ? 1 : 0);
		if (str.length() < len) return Err(TostringError::OutOfMemory);

		dyn_fmt_u32<PaddingStrategy::NoPadding>(StringRef(str.data(), len), uvalue, radix_count);

		if (is_negative) {
			str[0] = '-';
		}

		return Ok(len);
	}


};

struct FixedU32Formatter{
	[[nodiscard]] static constexpr 
	TostringResult<size_t> fmt(StringRef str, uint32_t value, Radix radix){
		const auto radix_count = radix.count();
	
		dyn_fmt_u32<PaddingStrategy::PaddingZero>(
			str, value, radix_count);

		return Ok(str.length());
	}

private:

};

struct Iq16Formatter{
	//TODO eps为5时计算会溢出 暂时限制eps=5的情况
	//TODO 支持除了Q16格式外其他格式转换到字符串的函数 

	static constexpr TostringResult<size_t> fmt(StringRef str, iq_t<16> value, const Eps eps_){
		if(str.length() == 0) return Err(TostringError::OutOfMemory);
		
		constexpr size_t Q = 16;

		const auto value_i32 = value.as_i32();
		const auto eps = MIN(eps_.count(), 4);

		const bool is_negtive = value_i32 < 0;
		const uint32_t abs_value = ABS(value_i32);
		constexpr uint32_t lower_mask = (Q == 31) ? 0x7fffffffu : uint32_t(((1 << Q) - 1));

		const uint32_t frac_part = uint32_t(abs_value) & lower_mask;

		const uint32_t scale = fast_exp10(eps);

		const uint32_t fs = frac_part * scale;
		
		const bool upper_round = (fs & lower_mask) >= (lower_mask >> 1);

		const uint32_t frac_int = (fs >> Q) + upper_round;
		const uint32_t digit_part = (uint32_t(abs_value) >> Q) + bool(frac_int >= scale);

		size_t len_acc = 0;

		if(is_negtive){
			str[0] = '-';
			len_acc += 1;
		}

		const auto digit_len = ({
			const auto res = IntFormatter<int>::fmt(
				str.substr_unchecked(len_acc), digit_part, Radix::Dec);
			if(res.is_err()) return Err(res.unwrap_err());
			res.unwrap();
		});

		len_acc += digit_len;

		//对小数点及后续的小数位格式化
		if(str.length() < size_t(len_acc + 1 + eps))
			return Err(TostringError::OutOfMemory);

		str[len_acc] = '.';
		len_acc += 1;

		//忽略返回的长度 因为它就是eps
		if(const auto res = FixedU32Formatter::fmt( 
			StringRef(str.data() + len_acc, eps), frac_int, Radix::Dec
		); res.is_err()) return Err(res.unwrap_err());
		
		return Ok(size_t(len_acc + eps));
	}
};

}

template<integral T>
static constexpr TostringResult<size_t> to_str(
	StringRef str, 
	T value, 
	Radix radix_count = Radix(Radix::Kind::Dec)
){
	return details::IntFormatter<T>::conv(str, value, radix_count);
}

template<size_t Q>
static constexpr TostringResult<size_t> to_str(StringRef str, iq_t<Q> value, const Eps eps = Eps(3)){
	return details::Iq16Formatter::fmt(str, value, eps);
}

template<size_t Q>
static constexpr DestringResult<iq_t<Q>> str_to_iq(StringView str){
	return details::IqDeformatter<iq_t<Q>>::defmt(str);
}

template<typename T>
struct DefmtStrDispatcher;

template<>
struct DefmtStrDispatcher<StringView>{
	static constexpr DestringResult<StringView> from_str(StringView str){
		return Ok(str);
	}
};

template<size_t Q>
struct DefmtStrDispatcher<iq_t<Q>>{
	static constexpr DestringResult<iq_t<Q>> from_str(StringView str){
		return str_to_iq<Q>(str);
	}
};

template<typename T>
requires std::is_integral_v<T>	
struct DefmtStrDispatcher<T>{
	static constexpr DestringResult<T> from_str(StringView str){
		return details::IntDeformatter<T>::defmt(str);
	}
};

template<typename T>
struct DefmtStrDispatcher<Angle<T>>{
	static constexpr DestringResult<Angle<T>> from_str(StringView str){
		if(const auto res = DefmtStrDispatcher<T>::from_str(str);
			res.is_err()) return Err(res.unwrap_err());
		else return Ok(Angle<T>::from_turns(res.unwrap()));
	}
};

template<typename T>
static constexpr DestringResult<T> defmt_str(StringView str){
	return DefmtStrDispatcher<T>::from_str(str);
}

}