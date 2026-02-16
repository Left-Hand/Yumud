#pragma once

#include "prelude.hpp"


#include "core/utils/Errno.hpp"
#include "core/utils/Result.hpp"


#include "core/string/view/string_view.hpp"

namespace ymd::strconv2{

#if 1
struct alignas(8) [[nodiscard]] FstrDump final{
	//整数部分的数值 eg: "12.34" => 12
	uint32_t digit_part;

	//小数部分的数值 eg: "12.34" => 34
	uint32_t frac_part;
	
	//是否为负数
	bool is_negative;

	//小数部分的位数 eg: "12.34" => 2
	uint8_t num_frac_digits;


	struct alignas(4) ParsingStatus{
		using Self = ParsingStatus;

		#if 0
		uint8_t has_dot:1;
		uint8_t has_digit_part:1;
		uint8_t has_frac_part:1;
		uint8_t is_negative:1;

		static constexpr Self from_default(){
			return std::bit_cast<Self>(uint8_t(0));
		}
		#else
		bool has_dot;
		bool has_digit_part;
		bool has_frac_part;
		char existing_sign;

		static constexpr Self from_default(){
			return std::bit_cast<Self>(uint32_t(0));
		}
		#endif


	};


	static constexpr DestringResult<FstrDump> parse(const StringView str) {
		if (str.length() == 0) {	
			return Err(DestringError::EmptyString);
		}

		uint64_t digit_part = 0;
		uint64_t frac_part = 0;
		uint8_t num_frac_digits = 0;

		ParsingStatus status = ParsingStatus::from_default();
		
		for(size_t ind = 0; ind < str.length(); ind++) {
			const char chr = str[ind];
			
			switch (chr) {
				case '\0':
					return Err(DestringError::NullTerminatorNotAllowed);
				case '0' ... '9':{

					const uint8_t digit = chr - '0';
					
					constexpr uint64_t MAX_INT_NUM = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max());
					if(status.existing_sign == '\0'){
						//如果还未找到符号就已经找到数字 那么已经隐含说明带有正号了
						status.existing_sign = '+';
					}

					if (status.has_dot == false) {
						status.has_digit_part = true;
						digit_part = digit_part * 10u + digit;
						// Check integer part overflow
						if (digit_part > MAX_INT_NUM) {
							return Err(DestringError::DigitOverflow);
						}
					} else {
						status.has_frac_part = true;
						frac_part = frac_part * 10u + digit;
						// Check fractional part overflow
						if (frac_part > MAX_INT_NUM) {
							return Err(DestringError::FracOverflow);
						}

						if(num_frac_digits < std::size(str::POW10_TABLE)){
							num_frac_digits++;
						}else{
							return Err(DestringError::FracTooLong);
						}
					}
					break;
				}

				case '+':
				case '-':{
					if(status.existing_sign != '\0'){
						if(chr == '-') return Err(DestringError::MultiplyNegative);
						else return Err(DestringError::MultiplyPositive);
					}
					status.existing_sign = chr;
					break;
				}
				case '.':  // Handle decimal dot
					if (status.has_dot) [[unlikely]]
						return Err(DestringError::MultipleDot);  // Multiple decimal dots
					status.has_dot = true;
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

		if(status.has_dot){
			//有小数点的情况不能没有小数部分
			if((status.has_frac_part == false)) [[unlikely]]
				return Err(DestringError::NoFracPart);
		}else{
			if (status.has_digit_part == false) [[unlikely]]
				return Err(DestringError::NoDigitPart);  // 符号位和小数点之间没有有效数字
		}


		return Ok(FstrDump{
			.digit_part = static_cast<uint32_t>(digit_part),
			.frac_part = static_cast<uint32_t>(frac_part),
			.is_negative = bool(status.existing_sign == '-'),
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

#else
struct alignas(8) [[nodiscard]] FstrDump final{
	//整数部分的数值 eg: "-12.34" => -12
	int32_t signed_digit_part;

	//小数部分的数值 eg: "12.34" => 34
	uint32_t frac_part;


	struct alignas(4) ParsingStatus{
		using Self = ParsingStatus;

		#if 0
		uint8_t has_dot:1;
		uint8_t has_digit_part:1;
		uint8_t has_frac_part:1;
		uint8_t is_negative:1;

		static constexpr Self from_default(){
			return std::bit_cast<Self>(uint8_t(0));
		}
		#else
		bool has_dot;
		bool has_digit_part;
		bool has_frac_part;
		char existing_sign;

		static constexpr Self from_default(){
			return std::bit_cast<Self>(uint32_t(0));
		}
		#endif


	};


	static constexpr DestringResult<FstrDump> parse(const StringView str) {
		if (str.length() == 0) {	
			return Err(DestringError::EmptyString);
		}

		uint64_t digit_part = 0;
		uint64_t frac_part = 0;
		uint8_t num_frac_digits = 0;

		ParsingStatus status = ParsingStatus::from_default();
		
		for(size_t ind = 0; ind < str.length(); ind++) {
			const char chr = str[ind];
			
			switch (chr) {
				case '\0':
					return Err(DestringError::NullTerminatorNotAllowed);
				case '0' ... '9':{

					const uint8_t digit = chr - '0';
					
					constexpr uint64_t MAX_INT_NUM = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max());
					if(status.existing_sign == '\0'){
						//如果还未找到符号就已经找到数字 那么已经隐含说明带有正号了
						status.existing_sign = '+';
					}

					if (status.has_dot == false) {
						status.has_digit_part = true;
						digit_part = digit_part * 10u + digit;
						// Check integer part overflow
						if (digit_part > MAX_INT_NUM) {
							return Err(DestringError::DigitOverflow);
						}
					} else {
						status.has_frac_part = true;
						frac_part = frac_part * 10u + digit;
						// Check fractional part overflow
						if (frac_part > MAX_INT_NUM) {
							return Err(DestringError::FracOverflow);
						}

						if(num_frac_digits < std::size(str::POW10_TABLE)){
							num_frac_digits++;
						}else{
							return Err(DestringError::FracTooLong);
						}
					}
					break;
				}

				case '+':
				case '-':{
					if(status.existing_sign != '\0'){
						if(chr == '-') return Err(DestringError::MultiplyNegative);
						else return Err(DestringError::MultiplyPositive);
					}
					status.existing_sign = chr;
					break;
				}
				case '.':  // Handle decimal dot
					if (status.has_dot) [[unlikely]]
						return Err(DestringError::MultipleDot);  // Multiple decimal dots
					status.has_dot = true;
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

		if(status.has_dot){
			//有小数点的情况不能没有小数部分
			if((status.has_frac_part == false)) [[unlikely]]
				return Err(DestringError::NoFracPart);
		}else{
			if (status.has_digit_part == false) [[unlikely]]
				return Err(DestringError::NoDigitPart);  // 符号位和小数点之间没有有效数字
		}


		return Ok(FstrDump{
			.digit_part = static_cast<uint32_t>(digit_part),
			.frac_part = static_cast<uint32_t>(frac_part),
		});
	}

	friend OutputStream & operator<<(OutputStream & os, const FstrDump & dump) {
		return os << dump.digit_part << os.splitter() 
			<< dump.frac_part
		;
	}
};
#endif
}