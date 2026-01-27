#pragma once

#include "prelude.hpp"


#include "core/utils/Errno.hpp"
#include "core/utils/Result.hpp"

#include "core/string/utils/pow10.hpp"
#include "core/string/view/string_view.hpp"

namespace ymd::strconv2{
struct [[nodiscard]] FstrDump final{
	uint32_t digit_part;
	uint32_t frac_part;
	bool is_negative;
	uint8_t num_frac_digits;


	struct ParsingStatus{
		using Self = ParsingStatus;

		uint8_t passed_dot:1;
		uint8_t contains_pre_dot_digits:1;
		uint8_t contains_post_dot_digits:1;
		uint8_t is_negative:1;

		static constexpr Self from_default(){
			return std::bit_cast<Self>(uint8_t(0));
		}
	};


	static constexpr DestringResult<FstrDump> defmt_from_str(const StringView str) {
		if (str.length() == 0) {	
			return Err(DestringError::EmptyString);
		}

		uint32_t digit_part = 0;
		uint32_t frac_part = 0;
		uint8_t num_frac_digits = 0;

		ParsingStatus status = ParsingStatus::from_default();
		const auto length = str.length();
		size_t index = 0;

		// 处理符号
		if (str[index] == '+' || str[index] == '-') {
			//如果字符串首字符为负号 说明这个数字是负数
			status.is_negative = (str[0] == '-');
			++ index;
			if (length == 1) {
				return Err(DestringError::OnlySignFounded);  // 只有符号没有数字
			}
		}

		while (index < length) {
			const char chr = str[index];
			
			switch (chr) {
				case 0:
					return Err(DestringError::UnexpectedZero);
				case '0' ... '9':{  // Digit case (GCC/Clang extension)

					const uint8_t digit = chr - '0';
					
					constexpr uint32_t MAX_INT_NUM = (std::numeric_limits<uint32_t>::max() - 9) / 10;
					if (status.passed_dot == false) {
						status.contains_pre_dot_digits = true;
						// Check integer part overflow
						if (digit_part > MAX_INT_NUM) {
							return Err(DestringError::DigitOverflow);
						}
						digit_part = digit_part * 10u + digit;
					} else {
						status.contains_post_dot_digits = true;
						// Check fractional part overflow
						if (frac_part > MAX_INT_NUM) {
							return Err(DestringError::FracOverflow);
						}
						frac_part = frac_part * 10u + digit;

						if(num_frac_digits < std::size(str::pow10_table)){
							num_frac_digits++;
						}else{
							return Err(DestringError::FracDigitsOverflow);
						}
					}
					++index;
					break;
				}

				case '+':
					return Err(DestringError::UnexpectedPositive);
				case '-':
					return Err(DestringError::UnexpectedNegative);
				case '.':  // Handle decimal dot
					if (status.passed_dot) [[unlikely]]
						return Err(DestringError::MultipleDot);  // Multiple decimal dots
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

		if(status.passed_dot){
			//有小数点的情况
			if((status.contains_post_dot_digits == false)) [[unlikely]]
				return Err(DestringError::NoDigitsAfterDot);
		}else{
			if (status.contains_pre_dot_digits == false) [[unlikely]]
				return Err(DestringError::NoDigitsAfterSign);  // 符号位和小数点之间没有有效数字
		}


		return Ok(FstrDump{
			.digit_part = digit_part,
			.frac_part = frac_part,
			.is_negative = bool(status.is_negative),
			.num_frac_digits = num_frac_digits
		});
	}

	friend OutputStream & operator<<(OutputStream & os, const FstrDump & dump) {
		return os << dump.digit_part << os.splitter() 
			<< dump.frac_part << os.splitter() 
			<< dump.is_negative << os.splitter() 
			<< dump.num_frac_digits
		;
	}
};
}