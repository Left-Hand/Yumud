#pragma once

#include <cstdint>
#include <type_traits>

namespace ymd::strconv2{

enum class DestringError:uint8_t{
    InvalidNumber,
    InvalidBooleanNum,         // 布尔值只能为0或1
    InvalidBooleanAlpha,       // 布尔值只能为true或false
    InvalidBooleanLength,      // 布尔值的长度必须为1,4,5
	InvalidBooleanChar,		   // 单符号布尔只能为'0','1'

    InvalidChar,            // Single character parsing failed
    InvalidDigit,

	UnexpectedPositive,
	UnexpectedNegative,
	MultiplyNegative,
	MultiplyPositive,

	MultipleDot,

	UnexpectedZero,
	UnexpectedSpace,
	UnexpectedChar,
	UnexpectedAlpha,
	NoDigit,
    PosOverflow,

    DigitOverflow,
	FracOverflow,

	NoDigits,
	OnlySignFounded,
	NoDigitsAfterSign,
	NoDigitsAfterDot,

	Overflow,
    NegOverflow, //负值超过能表示的范围
	NegForUnsigned, //负值不能用于无符号数
    EmptyString,

	BeginnerNotFounded,
	TerminatorNotFounded
};

DEF_DERIVE_DEBUG(DestringError)

enum class SerStringError:uint8_t{
	OutOfMemory
};

DEF_DERIVE_DEBUG(SerStringError)

template<typename T = void>
using DestringResult = Result<T,DestringError>;

template<typename T = void>
using SerStringResult = Result<T,SerStringError>;


struct [[nodiscard]] Radix final{
	enum class Kind:uint8_t{
		Dec = 10,
		Hex = 16,
		Bin = 2,
		Oct = 8,
	};

	using enum Kind;

	[[nodiscard]] constexpr Radix(const Kind kind):
		count_(static_cast<uint8_t>(kind)){;}

	[[nodiscard]] constexpr explicit Radix(const uint8_t count):
		count_(count){;}

	[[nodiscard]] constexpr uint8_t count() const{
		return count_;
	}

	[[nodiscard]] constexpr bool is_dec() const{
		return static_cast<Kind>(count_) == Dec;
	}

	[[nodiscard]] constexpr bool is_hex() const{
		return static_cast<Kind>(count_) == Hex;
	}

	[[nodiscard]] constexpr bool is_bin() const{
		return static_cast<Kind>(count_) == Bin;
	}

    [[nodiscard]] constexpr auto operator<=>(const Radix & other) const = default;

private:
	uint8_t count_;
};

struct [[nodiscard]] Eps final{
	uint8_t count_;

	constexpr explicit Eps(const uint8_t count):
		count_(count){;}

	[[nodiscard]] constexpr uint8_t count() const{
		return count_;
	}

    [[nodiscard]] constexpr auto operator<=>(const Eps & other) const = default;
private:

};

namespace details{

template<typename T>
struct chars_capacity_for_int;

template<typename T>
requires(std::is_integral_v<T> and (not std::is_same_v<T, bool>))
struct [[nodiscard]] chars_capacity_for_int<T>{
private:
    // 计算无符号整数类型的十进制位数
    [[nodiscard]] static consteval size_t num_digits(const Eps eps) {
        size_t digits = 0;
		using limits = std::numeric_limits<T>;
        T value = limits::max();
        do {
            ++digits;
            value /= eps.count();
        } while (value != 0);
        return digits;
    }

    // 计算有符号类型需要的字符数（包括符号位）
    [[nodiscard]] static consteval size_t from_signed(const Eps eps) {
        return 1 + // 符号位
			static_cast<size_t>(num_digits(eps)) + 1; // 数字位数
    }

    // 计算无符号类型需要的字符数
    [[nodiscard]] static consteval size_t from_unsigned(const Eps eps) {

        return 0 + //符号位
			static_cast<size_t>(num_digits(eps)) + 1;
    }
public:
	[[nodiscard]] static constexpr size_t value(const Eps eps){
		if constexpr(std::is_signed_v<T>) 
			return from_signed(eps);
		else 
			return from_unsigned(eps);
	}
};

template<>
struct [[nodiscard]] chars_capacity_for_int<bool>{
	static constexpr size_t NUM_FALSE_STR_LENGTH = 5;

	[[nodiscard]] static constexpr size_t value(const Eps eps){
		return NUM_FALSE_STR_LENGTH;
	}
};
}


template<typename T>
[[nodiscard]] static constexpr size_t chars_capacity_for_int_v(const Eps eps){
	return details::chars_capacity_for_int<T>::value(eps); 
}

}