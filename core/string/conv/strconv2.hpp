#pragma once

#include <cstdint>
#include <concepts>
#include <tuple>

#include "prelude.hpp"
#include "fstrdump.hpp"

#include "core/tmp/reflect/enum.hpp"
#include "core/tmp/exprimetal_integral.hpp"
#include "core/math/iq/fixed_t.hpp"

#include "core/string/view/mut_string_view.hpp"

#include "primitive/arithmetic/angular.hpp"

namespace ymd::strconv2 {

//是否为0~9的ascii字符
[[nodiscard]] __always_inline static constexpr 
bool is_digit_ascii(const char chr){
		return chr >= '0' && chr <= '9';}

///是否为a~z, A~Z的ascii字符
[[nodiscard]] __always_inline static constexpr 
bool is_alpha_ascii(const char chr) {
    return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z');
}

//数字转ascii字符
[[nodiscard]] __always_inline static constexpr 
char digit2char(uint8_t digit) noexcept {
	if (digit > 9) [[unlikely]]
		return digit + ('A' - 10);
	else 
		return digit + '0';
}

#if 0
//是否为整数或小数
[[nodiscard]] __always_inline static constexpr 
bool is_numeric(const StringView str) {
	const auto length = str.length();
	bool contains_digit = false;
	bool contains_dot = false;
	bool contains_sign = false;

	for (size_t i = 0; i < length; i++) {
		char chr = str[i];
		if(chr == '\0'){
			break;
		} else if (is_digit_ascii(chr)) {
			contains_digit = true;
		} else if (chr == '.') {
			if (contains_dot || !contains_digit) {
				return false;
			}
			contains_dot = true;
		} else if (chr == '+' || chr == '-') {
			if (contains_sign || contains_digit || contains_dot) {
				return false;
			}
			contains_sign = true;
		} else {
			return false;
		}
	}
	return contains_digit;
}


//是否为不含符号的整数
[[nodiscard]] __always_inline static constexpr 
bool is_digit_ascii(const StringView str){
	const auto length = str.length();
    for(size_t i = 0; i < length; i++){
		char chr = str[i];
        if(!is_digit_ascii(chr)) return false;
		if(chr == '\0') break;
    }
    return true;
}
#endif

namespace details{

template<integral T>
struct [[nodiscard]] IntDeformatter{
	using U = tmp::extended_unsigned_t<T>;
	static constexpr DestringResult<T> defmt(const StringView str) {

		const auto length = str.length();
		if(length == 0) return Err(DestringError::EmptyString);

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
			case '0' ... '9':
				break;
			default:
				return Err(DestringError::InvalidChar);
		}

		auto compute_final = [&] -> T{
			if constexpr(std::is_signed_v<T>){
				const auto ret_without_sign = static_cast<T>(unsigned_ret);
				return ((existing_sign == '-') ? -ret_without_sign : ret_without_sign);
			}else{
				return static_cast<T>(unsigned_ret);
			}
		};

		for(size_t i = size_t(bool(existing_sign)); i < length; i++){
			char chr = str[i];

			switch(chr){
				case '\0':
					return Ok(compute_final());
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
					unsigned_ret = (10u * unsigned_ret) + static_cast<U>(chr - '0');
					if((unsigned_ret > static_cast<U>(std::numeric_limits<T>::max()))) [[unlikely]]
						return Err(DestringError::Overflow);
					break;
				default:
					return Err(DestringError::InvalidChar);
			}
		}
		return Ok(compute_final());
	}
};

template<>
struct [[nodiscard]] IntDeformatter<bool>{
	static constexpr StringView TRUE_STR = StringView("true");
	static constexpr StringView FALSE_STR = StringView("false");
	static constexpr DestringResult<bool> defmt(const StringView str) {
		const auto length = str.length();
		switch(length){
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
		return Err(DestringError::InvalidBooleanLength);
	}
};

template<size_t NUM_Q, typename D>
struct [[nodiscard]] FixedPointDeformatter{
	using T = math::fixed_t<NUM_Q, D>;

	static constexpr size_t TABLE_LEN = std::size(str::pow10_table);
	static constexpr std::array<uint32_t, TABLE_LEN> TABLE = []{
		std::array<uint32_t, TABLE_LEN> ret;
		for(size_t i = 0; i < TABLE_LEN; i++){
			ret[i] = static_cast<uint32_t>(uint64_t(1 << NUM_Q) / str::pow10_table[i]);
		}
		return ret;
	}();

	static constexpr DestringResult<T> defmt(const StringView str){
		static_assert(sizeof(D) <= 4, "64bit is not supported yet");

		const auto dump = ({
			const auto res = FstrDump::from_str(str);
			if(res.is_err()) return Err(res.unwrap_err());
			res.unwrap();
		});
		
		if (dump.num_frac_digits == 0){
			return Ok(static_cast<T>(dump.digit_part));
		}else{
			const T frac_part = [&] -> T{
				return T::from_bits(
					static_cast<D>(int64_t(dump.frac_part) * TABLE[dump.num_frac_digits])
				);
			}();

			return Ok(frac_part + static_cast<T>(dump.digit_part));
		}

	}
};


template<size_t N>
__always_inline static constexpr std::tuple<uint32_t, uint8_t> fast_div(uint32_t x){
	if constexpr (std::has_single_bit(N)){
		constexpr size_t shift = __builtin_ctz(N);
		constexpr size_t mask = N - 1;
		return {x >> shift, uint8_t(x & mask)};
	}else{
		const auto q = iq16(1.0 / N);
		const auto r = uint32_t(x * q);
		return {r, uint8_t(x - r * N)};
	}
}

enum class DigitPaddingStrategy:uint8_t{
	NoPadding,
	ZeroPadded
};


//格式化u32到字符串(静态派发进制)
template<size_t Radix, DigitPaddingStrategy S>
__always_inline static constexpr void static_fmt_u32(
	MutStringView str, 
	uint32_t value
){
	size_t i = str.length();
	do {
		const auto [q, r] = fast_div<Radix>(value);
		str[--i] = digit2char(r);
		value = q;
	} while (i > 0 && value >= 0);

	if constexpr(S == DigitPaddingStrategy::ZeroPadded){
		while (i > 0) {
			str[--i] = '0';
		}
	}
}

//格式化u32到字符串(动态派发进制)
template<DigitPaddingStrategy S>
__always_inline static constexpr void dyn_fmt_u32(
	MutStringView str, 
	uint32_t int_val, 
	const uint8_t radix_count
){
	switch(radix_count){
		case 2: static_fmt_u32<2, S>(str, int_val); break;
		case 8: static_fmt_u32<8, S>(str, int_val); break;
		// case 10: static_fmt_u32<10, S>(str, int_val); break;
		case 16: static_fmt_u32<16, S>(str, int_val); break;
		
		default:{
			size_t i = str.length();
			do {
				uint8_t digit = int_val % radix_count;
				str[--i] = digit2char(digit);
				int_val /= radix_count;
			} while (i > 0 && int_val >= 0);

			if constexpr( S == DigitPaddingStrategy::ZeroPadded){
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
	SerStringResult<size_t> fmt(MutStringView str, const T int_val, const Radix radix){
		if (str.length() == 0)
			return Err(SerStringError::OutOfMemory);
			
		// Handle zero explicitly
		if ((int_val == 0)) [[unlikely]] {
			str[0] = '0';
			return Ok(1);
		}

		const auto radix_count = radix.count();
		
		if constexpr (std::is_signed_v<T>){
			const bool is_negative = int_val < 0;
			if (is_negative) {
				str[0] = '-';
			}

			// Make int_val positive
			using U = std::make_unsigned_t<T>;
			U unsigned_value = is_negative ? 
				static_cast<U>(-(int_val + 1)) + 1u : 
				static_cast<U>(int_val);

			// Compute length safely
			const size_t uint_length = str::uint_to_len_chars(unsigned_value, radix.count());
			const size_t total_length = uint_length + (is_negative ? 1 : 0);
			if (str.length() < total_length) return Err(SerStringError::OutOfMemory);

			dyn_fmt_u32<DigitPaddingStrategy::NoPadding>(
				MutStringView(str.data() + is_negative, uint_length), 
			unsigned_value, radix_count);

			return Ok(total_length);
		}else{
			const size_t total_length = uint_to_len_chars(int_val, radix);
			if (str.length() < total_length) return Err(SerStringError::OutOfMemory);
			dyn_fmt_u32<DigitPaddingStrategy::NoPadding>(
				MutStringView(str.data(), total_length), 
			int_val, radix_count);
			return Ok(total_length);
		}
	}
};

struct ZeroPaddedU32Formatter{
	[[nodiscard]] static constexpr 
	SerStringResult<size_t> fmt(MutStringView str, const uint32_t int_val, const Radix radix){
		const auto radix_count = radix.count();
	
		dyn_fmt_u32<DigitPaddingStrategy::ZeroPadded>(
			str, int_val, radix_count);

		return Ok(str.length());
	}

private:

};

struct Iq16Formatter{
	//TODO eps为5时计算会溢出 暂时限制eps=5的情况
	//TODO 支持除了Q16格式外其他格式转换到字符串的函数 
		
	static constexpr size_t Q = 16;
	static constexpr uint32_t lower_mask = (Q == 31) ? 0x7fffffffu : uint32_t(((1 << Q) - 1));
	static constexpr SerStringResult<size_t> fmt(
		MutStringView str,
		const math::fixed_t<16, int32_t> value, 
		const Eps eps
	){
		if(str.length() == 0) return Err(SerStringError::OutOfMemory);


		const auto value_i32 = value.to_bits();
		const auto eps_count = MIN(eps.count(), 4);

		const bool is_negative = value_i32 < 0;
		const uint32_t abs_value = ABS(value_i32);


		const uint32_t frac_part = uint32_t(abs_value) & lower_mask;

		const uint32_t scale = str::pow10_table[eps_count];

		const uint32_t fs = frac_part * scale;
		
		const bool upper_round = (fs & lower_mask) >= (lower_mask >> 1);

		const uint32_t frac_int = (fs >> Q) + upper_round;
		const uint32_t digit_part = (uint32_t(abs_value) >> Q) + bool(frac_int >= scale);

		size_t pos = 0;

		if(is_negative){
			str[0] = '-';
			pos += 1;
		}

		const auto digit_len = ({
			const auto res = IntFormatter<int>::fmt(
				str.substr_unchecked(pos), digit_part, Radix::Dec);
			if(res.is_err()) return Err(res.unwrap_err());
			res.unwrap();
		});

		pos += digit_len;

		//对小数点及后续的小数位格式化
		if(str.length() < size_t(pos + 1 + eps_count))
			return Err(SerStringError::OutOfMemory);

		str[pos] = '.';
		pos += 1;

		//忽略返回的长度 因为它就是eps
		if(const auto res = ZeroPaddedU32Formatter::fmt( 
			MutStringView(str.data() + pos, eps_count), frac_int, Radix::Dec
		); res.is_err()) return Err(res.unwrap_err());
		
		return Ok(size_t(pos + eps_count));
	}
};

}



template<typename T>
struct DefmtStrDispatcher;

template<>
struct DefmtStrDispatcher<StringView>{
	static constexpr DestringResult<StringView> from_str(StringView str){
		return Ok(str);
	}
};

template<size_t Q, typename D>
struct DefmtStrDispatcher<math::fixed_t<Q, D>>{
	static constexpr DestringResult<math::fixed_t<Q, D>> from_str(StringView str){
		return details::FixedPointDeformatter<Q, D>::defmt(str);
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
static constexpr DestringResult<T> defmt_from_str(StringView str){
	return DefmtStrDispatcher<T>::from_str(str);
}


template<integral T>
static constexpr SerStringResult<size_t> fmt_to_str(
	MutStringView str, 
	T value, 
	Radix radix_count = Radix(Radix::Kind::Dec)
){
	return details::IntFormatter<T>::conv(str, value, radix_count);
}

template<size_t Q>
static constexpr SerStringResult<size_t> fmt_to_str(MutStringView str, math::fixed_t<Q, int32_t> value, const Eps eps = Eps(3)){
	return details::Iq16Formatter::fmt(str, value, eps);
}


}