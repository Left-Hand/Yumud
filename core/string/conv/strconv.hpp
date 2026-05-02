#pragma once

#include <cstdint>
#include <concepts>
#include <tuple>

#include "core/tmp/reflect/enum.hpp"
#include "core/tmp/exprimetal_integral.hpp"
#include "core/math/fixed/fixed.hpp"
#include "core/string/view/mut_string_view.hpp"
#include "core/string/utils/pow10.hpp"

#include "prelude.hpp"
#include "fstrdump.hpp"


#include "primitive/arithmetic/angular.hpp"

namespace ymd::strconv {

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
	if (digit > 9)
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


struct [[nodiscard]] IntDeformatterGeneric{
	static constexpr DestringResult<Radix> parse_radix(const StringView str){
		const auto str_length = str.length();
		if(str_length < 2) return Ok(Radix(Radix::Kind::Dec));

		// "0x", "0b", "0o"

		if(str[0] != '0') return Ok(Radix(Radix::Kind::Dec));
		switch(str[1]){
			case 'x':
			case 'X':
				if(str_length <= 2) return Err(DeformatError::HexBaseOnly);
				return Ok(Radix(Radix::Kind::Hex));
				break;
			case 'b': 
			case 'B':
				if(str_length <= 2) return Err(DeformatError::BinBaseOnly);
				return Ok(Radix(Radix::Kind::Bin));
				break;
			case 'o': 
			case 'O':
				if(str_length <= 2) return Err(DeformatError::OctBaseOnly);
				return Ok(Radix(Radix::Kind::Oct));
				break;
			case '0' ... '9':
				return Ok(Radix(Radix::Kind::Dec));
				break;
			default:
				return Err(DeformatError::InvalidChar);
		}
	}


	template<typename T>
	static constexpr DestringResult<T> parse_dec(const StringView str) {
		using U = std::conditional_t<sizeof(T) >= 4, uint64_t, uint32_t>;
		const auto length = str.length();
		if(length == 0) return Err(DeformatError::EmptyString);

		U unsigned_temp = 0;
		char existing_sign = '\0';

		auto compute_final = [&] -> T{
			if constexpr(std::is_signed_v<T>){
				const T unsigned_ret = static_cast<T>(unsigned_temp);
				return ((existing_sign == '-') ? (-unsigned_ret) : (unsigned_ret));
			}else{
				return static_cast<T>(unsigned_temp);
			}
		};

		for(size_t i = size_t(bool(existing_sign)); i < length; i++){
			char chr = str[i];

			switch(chr){
				case '\0':
					return Ok(compute_final());
				case '-':
					if constexpr(std::is_unsigned_v<T>)
						return Err(DeformatError::NegForUnsigned);
					if(existing_sign != 0) 
						return Err(DeformatError::MultiplyNegative);
					existing_sign = '-';
					break;
				case '+':
					if(existing_sign != 0) 
						return Err(DeformatError::MultiplyPositive);
					existing_sign = '+';
					break;
				case '0' ... '9':{
					if(existing_sign == '\0'){
						//如果还未找到符号就已经找到数字 那么已经隐含说明带有正号了
						existing_sign = '+';
					}

					unsigned_temp = (10u * unsigned_temp) + static_cast<U>(chr - '0');
					{
						constexpr U MAX_INT_NUM = static_cast<U>(std::numeric_limits<T>::max());
						if constexpr(std::is_signed_v<T>){
							if((existing_sign == '-') and (unsigned_temp > MAX_INT_NUM + 1)) [[unlikely]]
								return Err(DeformatError::Underflow);
						}
						if((unsigned_temp > MAX_INT_NUM)) [[unlikely]]
							return Err(DeformatError::Overflow);
					}

					break;
				}
				case '.':
					return Err(DeformatError::UnexpectedDotInInteger);
				case 'a' ... 'z':
				case 'A' ... 'Z':
					return Err(DeformatError::UnexpectedAlpha);
				default:
					return Err(DeformatError::InvalidChar);
			}
		}
		return Ok(compute_final());
	}

	template<typename T>
	static constexpr DestringResult<T> parse_bare_hex(const StringView str) {
		using U = std::conditional_t<sizeof(T) >= 4, uint64_t, uint32_t>;
		static_assert(std::is_unsigned_v<T>, "hex must be unsigned");

		const auto str_length = str.length();
		if(str_length == 0) return Err(DeformatError::EmptyString);

		U temp = 0;

		//nibble calc
		if(str_length > sizeof(T) * 2) return Err(DeformatError::StrTooLong);

		#pragma GCC unroll 2
		for(size_t i = 0; i < str_length; i ++){
			const auto chr = str[i];

			uint8_t nibble = 0;
			switch(chr){
				case '\0':
					return Err(DeformatError::InvalidNullTerminator);
				case '0' ... '9':
					nibble = chr - '0';
					break;
				case 'a' ... 'f':
					nibble = chr - 'a' + 10;
					break;

				case 'A' ... 'F':
					nibble = chr - 'A' + 10;
					break;
				default:
					return Err(DeformatError::InvalidChar);
			}

			temp = (temp << 4) | nibble;
		}

		return Ok(static_cast<T>(temp));
	}

	template<typename T>
	static constexpr DestringResult<T> parse_bare_oct(const StringView str) {
		using U = std::conditional_t<sizeof(T) >= 4, uint64_t, uint32_t>;
		static_assert(std::is_unsigned_v<T>, "oct must be unsigned");

		const auto str_length = str.length();
		if(str_length == 0) return Err(DeformatError::EmptyString);

		U temp = 0;

		// 八进制：每个字符3位，但需要处理前导零
		// 最大长度 = ceil(sizeof(T) * 8 / 3)
		constexpr size_t MAX_OCT_DIGITS = (sizeof(T) * 8 + 2) / 3; // 向上取整
		if(str_length > MAX_OCT_DIGITS) return Err(DeformatError::StrTooLong);

		#pragma GCC unroll 2
		for(size_t i = 0; i < str_length; i++) {
			const auto chr = str[i];

			uint8_t digit = 0;
			switch(chr) {
				case '\0':
					return Err(DeformatError::InvalidNullTerminator);
				case '0' ... '7':
					digit = chr - '0';
					break;
				case '8' ... '9':
					return Err(DeformatError::DigitExceedsOct);
				default:
					return Err(DeformatError::InvalidChar);
			}

			// 检查移位是否会导致溢出（对于U可能小于T的情况）
			if constexpr (sizeof(U) * 8 <= sizeof(T) * 8) {
				// 如果U不比T大，需要检查移位溢出
				constexpr U max_shift_safe = std::numeric_limits<U>::max() >> 3;
				if (temp > max_shift_safe) {
					return Err(DeformatError::Overflow);
				}
			}
			
			temp = (temp << 3) | digit;
		}

		// 最终检查是否适合T
		if (temp > std::numeric_limits<T>::max()) {
			return Err(DeformatError::Overflow);
		}

		return Ok(static_cast<T>(temp));
	}

	template<typename T>
	static constexpr DestringResult<T> parse_bare_bin(const StringView str) {
		using U = std::conditional_t<sizeof(T) >= 4, uint64_t, uint32_t>;
		static_assert(std::is_unsigned_v<U>, "bin must be unsigned");

		const auto str_length = str.length();
		if(str_length == 0) return Err(DeformatError::EmptyString);

		U temp = 0;

		// 二进制：每个字符1位
		if(str_length > sizeof(T) * 8) return Err(DeformatError::StrTooLong);

		#pragma GCC unroll 2
		for(size_t i = 0; i < str_length; i++) {
			const auto chr = str[i];

			uint8_t bit = 0;
			switch(chr) {
				case '\0':
					return Err(DeformatError::InvalidNullTerminator);
				case '0' ... '1':
					bit = chr - '0';
					break;
				case '2' ... '9':
					return Err(DeformatError::DigitExceedsBin);
				default:
					return Err(DeformatError::InvalidChar);
			}
			
			temp = (temp << 1) | bit;
		}

		return Ok(static_cast<T>(temp));
	}
};

template<integral T>
struct [[nodiscard]] IntDeformatter final:public IntDeformatterGeneric{
	using U = std::conditional_t<sizeof(T) >= 4, uint64_t, uint32_t>;
	static constexpr DestringResult<T> parse(StringView str){
		if constexpr(std::is_signed_v<T>){
			return parse_dec<T>(str);
		}else{
			//只有无符号数才可能检测为hex/oct/bin

			const Radix radix = ({
				const auto may_radix = parse_radix(str);
				if(may_radix.is_err()) return Err(may_radix.unwrap_err());
				may_radix.unwrap();
			});

			if((radix.count() != 10) and (str.length() > 2)){
				//移除前缀
				str = StringView{str.data() + 2, str.size() - 2};
			}

			switch(radix.count()){
				case 2: return parse_bare_bin<T>(str);
				case 8: return parse_bare_oct<T>(str);
				case 10: return parse_dec<T>(str);
				case 16: return parse_bare_hex<T>(str);
			}

			return Err(DeformatError::InvalidRadix);
		}

	}
};

template<>
struct [[nodiscard]] IntDeformatter<bool> final{
	static constexpr StringView TRUE_STR = StringView("true");
	static constexpr StringView FALSE_STR = StringView("false");
	static constexpr DestringResult<bool> parse(const StringView str) {
		const auto length = str.length();
		switch(length){
			case 1:
				switch(str[0]){
					case '0':
						return Ok(false);
					case '1':
						return Ok(true);
					case '-':
						return Err(DeformatError::NegativeBoolean);
					case '+':
						return Err(DeformatError::PositiveBoolean);
					default:	
						return Err(DeformatError::InvalidBooleanChar);
				}
			case 4:
				if(str != TRUE_STR)
					return Err(DeformatError::InvalidBooleanAlpha);
				return Ok(true);
			case 5:
				if(str != FALSE_STR)
					return Err(DeformatError::InvalidBooleanAlpha);
				return Ok(false);
		}
		return Err(DeformatError::InvalidBooleanLength);
	}
};


template<size_t NUM_Q, typename D>
struct [[nodiscard]] FixedPointSynthesizer final{

	using T = math::fixed<NUM_Q, D>;

	static constexpr size_t TABLE_LEN = std::size(str::POW10_TABLE);
	static constexpr uint32_t DIGIT_MAX = uint32_t(
		std::numeric_limits<math::fixed<NUM_Q, D>>::max());	

	static constexpr std::array<uint64_t, TABLE_LEN> INV_POW10_TABLE = []{
		std::array<uint64_t, TABLE_LEN> ret;
		for(size_t i = 0; i < TABLE_LEN; i++){
			ret[i] = static_cast<uint64_t>(uint64_t(1ull << (NUM_Q + 32u)) / str::POW10_TABLE[i]);
		}
		return ret;
	}();

	static constexpr DestringResult<T> synthesize(
		const FstrDump & dump
	){
		auto apply_sign = [&](const T x) -> T{
			if constexpr(std::is_unsigned_v<T>){
				return x;
			}else{
				if(dump.is_negative) return -x;
				return x;
			}
		};
		
		if (dump.num_frac_digits == 0){
			return Ok(apply_sign(static_cast<T>(dump.digit_part)));
		}else{
			if(dump.num_frac_digits >= TABLE_LEN){
				return Err(DeformatError::FracTooLong);
			}

			const T res = T::from_bits(
				static_cast<D>((uint64_t(dump.frac_part) * INV_POW10_TABLE[dump.num_frac_digits]) >> 32)
			) + static_cast<T>(dump.digit_part);

			return Ok(apply_sign(res));
		}
	}
};

template<size_t NUM_Q, typename D>
struct [[nodiscard]] FixedPointDeformatter final{
	using T = math::fixed<NUM_Q, D>;

	static constexpr uint32_t DIGIT_MAX = uint32_t(
		std::numeric_limits<math::fixed<NUM_Q, D>>::max());	
	static constexpr DestringResult<T> parse(const StringView str){
		static_assert(sizeof(D) <= 4, "64bit is not supported yet");

		//从字符串dump出相关信息
		const FstrDump dump = ({
			const auto res = FstrDump::parse(str);
			if(res.is_err()) return Err(res.unwrap_err());
			res.unwrap();
		});


		//进行防溢出检查
		if constexpr(std::is_unsigned_v<D>){
			if(dump.is_negative) return Err(DeformatError::NegForUnsigned);
			if(dump.digit_part > DIGIT_MAX) return Err(DeformatError::Overflow);
		}else{
			if(dump.is_negative){
				if(dump.digit_part > static_cast<uint32_t>(DIGIT_MAX + 1u)){
					return Err(DeformatError::Underflow);
				}
			}else{
				if(dump.digit_part > DIGIT_MAX) return Err(DeformatError::Overflow);
			}
		}

		return FixedPointSynthesizer<NUM_Q, D>::synthesize(
			dump
		);

	}
};

struct [[nodiscard]] FloatDeformatter final{
	using T = float;

	static constexpr size_t TABLE_LEN = 12;
	static constexpr std::array<T, TABLE_LEN> TABLE = []{
		std::array<T, TABLE_LEN> ret;
		uint64_t ratio = 1;
		for(size_t i = 0; i < TABLE_LEN; i++){
			ret[i] = static_cast<T>(static_cast<long double>(1) / ratio);
			ratio *= 10u;
		}
		return ret;
	}();

	static constexpr DestringResult<T> parse(const StringView str){
		//从字符串dump出相关信息
		const FstrDump dump = ({
			const auto res = FstrDump::parse(str);
			if(res.is_err()) return Err(res.unwrap_err());
			res.unwrap();
		});

		auto conv_ret = [&](const T unsigned_ret) -> T{
			if(dump.is_negative) return -static_cast<T>(unsigned_ret);
			return static_cast<T>(unsigned_ret);
		};
		
		if (dump.num_frac_digits == 0){
			return Ok(static_cast<T>(conv_ret(dump.digit_part)));
		}else{
			const T frac_part = dump.frac_part * TABLE[dump.num_frac_digits];
			return Ok(conv_ret(frac_part + static_cast<T>(dump.digit_part)));
		}
	}
};


#if 0


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
	} while (i > 0);

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
			} while (i > 0);

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
	SerStringResult<size_t> fmt_to_str(MutStringView str, const T int_val, const Radix radix){
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
	SerStringResult<size_t> fmt_to_str(MutStringView str, const uint32_t int_val, const Radix radix){
		const auto radix_count = radix.count();
	
		dyn_fmt_u32<DigitPaddingStrategy::ZeroPadded>(
			str, int_val, radix_count);

		return Ok(str.length());
	}

private:

};

#endif

template<typename T>
struct DefmtStrDispatcher;

template<>
struct DefmtStrDispatcher<StringView>{
	static constexpr DestringResult<StringView> defmt_from_str(StringView str){
		return Ok(str);
	}
};

template<size_t Q, typename D>
struct DefmtStrDispatcher<math::fixed<Q, D>>{
	static constexpr DestringResult<math::fixed<Q, D>> defmt_from_str(StringView str){
		return FixedPointDeformatter<Q, D>::parse(str);
	}
};

template<typename T>
requires std::is_integral_v<T>	
struct DefmtStrDispatcher<T>{
	static constexpr DestringResult<T> defmt_from_str(StringView str){
		return IntDeformatter<T>::parse(str);
	}
};

template<typename T>
requires std::is_floating_point_v<T>	
struct DefmtStrDispatcher<T>{
	static constexpr DestringResult<T> defmt_from_str(StringView str){
		const auto res = FloatDeformatter::parse(str);
		if(res.is_err()) return Err(res.unwrap_err());
		return Ok(res.unwrap());
	}
};


template<typename T>
static constexpr DestringResult<T> defmt_from_str(StringView str){
	return DefmtStrDispatcher<T>::defmt_from_str(str);
}


// template<integral T>
// static constexpr SerStringResult<size_t> fmt_to_str(
// 	MutStringView str, 
// 	T value, 
// 	Radix radix = Radix(Radix::Kind::Dec)
// ){
// 	return IntFormatter<T>::fmt_to_str(str, value, radix);
// }

// template<size_t Q>
// static constexpr SerStringResult<size_t> fmt_to_str(MutStringView str, math::fixed<Q, int32_t> value, const Eps eps = Eps(3)){
// 	return Iq16Formatter::fmt_to_str(str, value, eps);
// }


}