#pragma once

#include <cstdint>
#include <type_traits>

namespace ymd::strconv2{

enum class DestringError:uint8_t{
    InvalidNumber,
    InvalidBoolean,         // Boolean parsing failed
    InvalidChar,            // Single character parsing failed
    InvalidDigit,
	UnexpectedPositive,
	UnexpectedNegative,
	MultipleDot,
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
    NegOverflow,
    EmptyString,
};

DERIVE_DEBUG(DestringError)

enum class TostringError:uint8_t{
	OutOfMemory
};

DERIVE_DEBUG(TostringError)

template<typename T = void>
using DestringResult = Result<T,DestringError>;

template<typename T = void>
using TostringResult = Result<T,TostringError>;


struct Radix final{
	enum class Kind:uint8_t{
		Dec = 10,
		Hex = 16,
		Bin = 2,
		Oct = 8,
	};

	using enum Kind;

	constexpr Radix(const Kind kind):
		count_(static_cast<uint8_t>(kind)){;}

	constexpr explicit Radix(const uint8_t count):
		count_(count){;}

	constexpr uint8_t count() const{
		return count_;
	}

	constexpr bool is_dec() const{
		return static_cast<Kind>(count_) == Dec;
	}

	constexpr bool is_hex() const{
		return static_cast<Kind>(count_) == Hex;
	}

	constexpr bool is_bin() const{
		return static_cast<Kind>(count_) == Bin;
	}

    constexpr auto operator<=>(const Radix & other) const = default;

private:
	uint8_t count_;
};

struct Eps final{
	constexpr explicit Eps(const uint8_t count):
		count_(count){;}

	constexpr uint8_t count() const{
		return count_;
	}

    constexpr auto operator<=>(const Eps & other) const = default;
private:
	uint8_t count_;
};

template<typename T>
struct str_buffer_capacity{
	
};

template<typename T>
requires(std::is_integral_v<T>)
struct str_buffer_capacity<T>{
    // 计算有符号类型需要的字符数（包括符号位）
    static constexpr size_t for_signed() {
        using limits = std::numeric_limits<T>;
        return 1 + // 符号位
               static_cast<size_t>(digits10(limits::max())) + 1; // 数字位数
    }

    // 计算无符号类型需要的字符数
    static constexpr size_t for_unsigned() {
        using limits = std::numeric_limits<T>;
        return static_cast<size_t>(digits10(limits::max())) + 1;
    }

    static constexpr auto value = 
        std::numeric_limits<T>::is_signed ? for_signed() : for_unsigned();
private:
    // 计算无符号整数类型的十进制位数
    static constexpr size_t digits10() noexcept {
        size_t digits = 0;
        T value = std::numeric_limits<T>::max();
        do {
            ++digits;
            value /= 10;
        } while (value != 0);
        return digits;
    }

};

template<typename T>
static constexpr size_t str_buffer_capacity_v = str_buffer_capacity<T>::value; 

}