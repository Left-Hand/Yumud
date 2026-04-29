#pragma once

#include "core/string/conv/strconv2.hpp"
#include "core/string/utils/split_iter.hpp"
#include "core/utils/result.hpp"

namespace ymd::gcode{


enum class [[nodiscard]] GcodeParseError:uint8_t{
    NoLetterFounded,
    NoStringSegmentFounded,
    NoMajorNumber,
    NoMinorNumber,
    MajorNumberOverflow,
    MinorNumberOverflow,
    UnknownHeadLetter,
    NoMnemonicFounded,
    EmptyArg,
    InvalidMnemonic,
    InvalidArgumentLetter
};

DEF_DERIVE_DEBUG(GcodeParseError)

DEF_ERROR_WITH_KINDS(Error, GcodeParseError, strconv2::DeformatError)

template<typename T = void>
using IResult = Result<T, Error>;   

struct [[nodiscard]] Mnemonic final{
    enum class [[nodiscard]] Kind:uint8_t {
        General,
        Miscellaneous,
        ProgramNumber,
        ToolChange,
    };

    DEF_FRIEND_DERIVE_DEBUG(Kind);

    [[nodiscard]] static constexpr Option<Mnemonic> try_from_letter(const char letter){
        switch(letter){
            case 'G':
                return Some(Mnemonic{Kind::General});
            case 'M':
                return Some(Mnemonic{Kind::Miscellaneous});
            case 'T':
                return Some(Mnemonic{Kind::ToolChange});
            case 'N':
                return Some(Mnemonic{Kind::ProgramNumber});
        }
        return None;
    }

    template<char letter>
    [[nodiscard]] static consteval Mnemonic from_letter(){
        return try_from_letter(letter).unwrap();
    }


    [[nodiscard]] static constexpr bool is_letter_valid(const char letter){
        switch(letter){
            case 'G':
            case 'M':
            case 'T':
            case 'N':
                return true;
            default: 
                return false;
        }
    }

    constexpr Mnemonic(const Kind kind):
        kind_(kind){;}

    [[nodiscard]] constexpr bool operator==(const Mnemonic & other) const{
        return kind_ == other.kind_;
    }

    [[nodiscard]] constexpr bool operator==(const Kind kind) const {
        return kind_ == kind;
    }

    [[nodiscard]] constexpr Kind kind() const {
        return kind_;
    }

    [[nodiscard]] constexpr char to_letter() const {
        switch(kind_){
            case Kind::General:
                return 'G';
            case Kind::Miscellaneous:
                return 'M';
            case Kind::ProgramNumber:
                return 'P';
            case Kind::ToolChange:
                return 'T';
        }
        //anyone can't reach here
        __builtin_unreachable();
    }

    constexpr Mnemonic(const Mnemonic & other) = default; 
    constexpr Mnemonic(Mnemonic && other) = default; 
private:
    Kind kind_;
public:
    using enum Kind;

    friend OutputStream & operator<<(OutputStream & os, const Mnemonic self){
        return os << self.kind();
    }
};

struct [[nodiscard]] TextSourceLocation final{
    uint8_t start;    // Starting column
    uint8_t end;      // Ending column  
    uint16_t line;    // Line number
    
    friend OutputStream& operator<<(OutputStream& os, const TextSourceLocation self) {
        return os << os.brackets<'('>() 
            << self.start << os.splitter()
            << self.end << os.splitter()
            << self.line
            << os.brackets<')'>();
    }
};


struct GcodeValue{
    using Self = GcodeValue;

    static constexpr uint16_t DIGIT_MAX = std::numeric_limits<uint16_t>::max();

    struct [[nodiscard]] Specifiers final{
        uint8_t has_dot:1;
        uint8_t has_digit_part:1;
        uint8_t has_frac_part:1;
        uint8_t is_negative:1;
        uint8_t num_frac_digits:4;
        char existing_sign;

        static constexpr Specifiers from_default(){
            return std::bit_cast<Specifiers>(uint16_t(0));
        }
    };



    uint16_t digit_part_;
    Specifiers specifiers_;
    uint32_t frac_part_;

    constexpr uint32_t unsigned_digit() const {
        return static_cast<uint32_t>(digit_part_);
    }

    constexpr uint32_t signed_digit() const {
        auto temp = static_cast<int32_t>(digit_part_);
        if(specifiers_.existing_sign == '-') return -temp;
        return temp;
    }

    constexpr Option<uint32_t> frac() const {
        if(not specifiers_.has_frac_part) return None;
        return Some(static_cast<uint32_t>(digit_part_));
    }


    template<std::floating_point T>
    constexpr T to_floating() const {
        T temp = static_cast<T>(frac_part_);
        for(size_t i = 0; i < static_cast<size_t>(specifiers_.num_frac_digits); i++){
            temp *= static_cast<T>(0.1);
        }

        temp += static_cast<T>(digit_part_);
        if(specifiers_.is_negative) temp = -temp;
        return temp;
    }


    constexpr float to_f32() const {
        return to_floating<float>();
    }

    constexpr double to_f64() const {
        return to_floating<double>();
    }


    template<size_t NUM_Q, typename D>
    constexpr math::fixed<NUM_Q, D> to_fixed() const {
        constexpr size_t TABLE_LEN = std::size(str::POW10_TABLE);

        constexpr std::array<uint64_t, TABLE_LEN> TABLE = []{
            std::array<uint64_t, TABLE_LEN> ret;
            for(size_t i = 0; i < TABLE_LEN; i++){
                ret[i] = static_cast<uint64_t>(uint64_t(1ull << (NUM_Q + 32u)) / str::POW10_TABLE[i]);
            }
            return ret;
        }();

        using T = math::fixed<NUM_Q, D>;
        const int32_t digit_part = static_cast<int32_t>(signed_digit());
        const size_t num_frac_digits = static_cast<size_t>(specifiers_.num_frac_digits);
		auto conv_ret = [&](const T unsigned_ret) -> T{
			if constexpr(std::is_unsigned_v<T>){
				return unsigned_ret;
			}else{
				if(digit_part < 0) return -unsigned_ret;
				return unsigned_ret;
			}
		};
		
		if (num_frac_digits == 0){
			return static_cast<T>(conv_ret(digit_part));
		}else{

			const T frac = [&] -> T{
				return T::from_bits(
					static_cast<D>((uint64_t(frac_part_) * TABLE[num_frac_digits]) >> 32)
				);
			}();

			return conv_ret(frac + static_cast<T>(digit_part));
		}
    };

    template<typename T>
    constexpr T to_numeric() const {
        if constexpr(std::is_floating_point_v<T>){
            return to_floating<T>();
        }else if constexpr(tmp::is_fixed_point_v<T>){
            static constexpr size_t Q_NUM = tmp::fixed_point_qnum_v<T>;
            return to_fixed<
                Q_NUM, 
                tmp::fixed_point_underlying_type_t<T>
            >();
        }
        __builtin_unreachable();
    }

    static constexpr Result<GcodeValue, Error> try_from_str(const StringView str){
        using namespace strconv2;

		if (str.length() == 0) {	
			return Err(DeformatError::EmptyString);
		}

		uint64_t digit_part = 0;
		uint64_t frac_part = 0;
		uint8_t num_frac_digits = 0;

		Specifiers specifiers = Specifiers::from_default();
		
		for(size_t ind = 0; ind < str.length(); ind++) {
			const char chr = str[ind];
			
			switch (chr) {
				case '\0':
					return Err(DeformatError::InvalidNullTerminator);
				case '0' ... '9':{

					const uint8_t digit = chr - '0';
					
					constexpr uint64_t MAX_INT_NUM = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max());
					if(specifiers.existing_sign == '\0'){
						//如果还未找到符号就已经找到数字 那么已经隐含说明带有正号了
						specifiers.existing_sign = '+';
					}

					if (specifiers.has_dot == false) {
						specifiers.has_digit_part = true;
						digit_part = digit_part * 10u + digit;
						// Check integer part overflow
						if (digit_part > MAX_INT_NUM) {
							return Err(DeformatError::DigitOverflow);
						}
					} else {
						specifiers.has_frac_part = true;
						frac_part = frac_part * 10u + digit;
						// Check fractional part overflow
						if (frac_part > MAX_INT_NUM) {
							return Err(DeformatError::FracOverflow);
						}

						if(num_frac_digits < std::size(str::POW10_TABLE)){
							num_frac_digits++;
						}else{
							return Err(DeformatError::FracTooLong);
						}
					}
					break;
				}

				case '+':
				case '-':{
					if(specifiers.existing_sign != '\0'){
						if(chr == '-') return Err(DeformatError::MultiplyNegative);
						else return Err(DeformatError::MultiplyPositive);
					}
					specifiers.existing_sign = chr;
					break;
				}
				case '.':  // Handle decimal dot
					if (specifiers.has_dot) [[unlikely]]
						return Err(DeformatError::MultipleDot);  // Multiple decimal dots
					specifiers.has_dot = true;
					break;
				case 'a' ... 'z':
					return Err(DeformatError::UnexpectedAlpha);
				case 'A' ... 'Z':
					return Err(DeformatError::UnexpectedAlpha);
				case ' ':
					return Err(DeformatError::UnexpectedSpace);
				default:  // Invalid characters
					return Err(DeformatError::UnexpectedChar);
			}

		}

		if(specifiers.has_dot){
			//有小数点的情况不能没有小数部分
			if((specifiers.has_frac_part == false)) [[unlikely]]
				return Err(DeformatError::NoFracPart);
		}else{
			if (specifiers.has_digit_part == false) [[unlikely]]
				return Err(DeformatError::NoDigitPart);  // 符号位和小数点之间没有有效数字
		}


        specifiers.num_frac_digits = num_frac_digits;


		return Ok(Self{
			.digit_part_ = static_cast<uint16_t>(digit_part),
			.specifiers_ = specifiers,
			.frac_part_ = static_cast<uint32_t>(frac_part),
		});

    };

    friend OutputStream & operator << (OutputStream & os, const Self & self){
        os << self.signed_digit();
        if(self.specifiers_.has_frac_part){
            os << '.' << self.frac_part_;
        }
        return os;
    }
};

struct [[nodiscard]] GcodeWord final{
    using Self = GcodeWord;

    char letter;
    GcodeValue value;

    [[nodiscard]] static constexpr bool is_letter_valid(const char letter){
        switch(letter){
            case 'X':
            case 'Y':
            case 'F':
                return true;

            case 'G':
            case 'M':
            case 'P':
            case 'T':
                return true;

            default: 
                return false;
        }
    }

    static constexpr Result<GcodeWord, Error> try_from_str(const StringView str){
    
        const char letter = str[0];
        // if (Mnemonic::is_letter_valid(letter))
        //     continue;
        if (!GcodeWord::is_letter_valid(letter)) {
            // Invalid letter (e.g., "@100") -> return Error
            return Err(GcodeParseError::InvalidArgumentLetter);
        }

        // Parse value (skip letter)
        const auto value_str = StringView(std::next(str.begin()), str.end());


        return Ok(Self{
            .letter = letter,
            .value = ({
                const auto res = GcodeValue::try_from_str(value_str);
                if(res.is_err()) return Err(res.unwrap_err());
                res.unwrap();
            })
        });

    } 

    friend OutputStream & operator << (OutputStream & os, const GcodeWord & self){
        return os << os.brackets<'('>() 
            << self.letter << os.splitter()
            << self.value << os.brackets<')'>();
    }
};


struct [[nodiscard]] GcodeWordsIter final{
    constexpr explicit GcodeWordsIter(StringView line)
        : arg_str_iter_(line, ' ') {}

    [[nodiscard]] constexpr bool has_next() const {
        // No more arguments -> return None
        return arg_str_iter_.has_next();
    }

    constexpr IResult<GcodeWord> next() {
        if(not (arg_str_iter_.has_next())){
            __builtin_trap();
        }

        const auto token_str = arg_str_iter_.next();
        if (token_str.length() == 0) return Err(GcodeParseError::EmptyArg); // Skip empty tokens (e.g., multiple spaces)

        return GcodeWord::try_from_str(token_str);

    }

private:
    StringSplitIter arg_str_iter_;
};


namespace details{
constexpr IResult<StringView> query_value_str(const StringView line, const char letter) {
    auto value_position = ({
        const auto may_index = line.find(letter);
        if(may_index.is_none()) 
            return Err(GcodeParseError::NoLetterFounded);
        may_index.unwrap();
    });

    auto iter = StringSplitIter(
        line.substr(value_position).unwrap(), ' '
    );

    if(not iter.has_next()) 
        return Err(GcodeParseError::NoStringSegmentFounded);
    return Ok(iter.next());
}

template<typename T, typename FnMap>
constexpr IResult<T> query_tmp(const StringView line, const char letter, FnMap && fn) {
    if(const auto either_str = query_value_str(line, letter);
        either_str.is_err()) return Err(either_str.unwrap_err());
    else 
        return std::forward<FnMap>(fn)(either_str.unwrap());
}
}

struct [[nodiscard]] GcodeLine final{
    StringView line;
    constexpr IResult<Mnemonic> query_mnemonic() const {
        if(line.length() == 0) 
            return Err(GcodeParseError::NoMnemonicFounded);

        const auto may_mnemoic = Mnemonic::try_from_letter(line[0]);
        if(may_mnemoic.is_none()) 
            return Err(GcodeParseError::InvalidMnemonic);
        return Ok(may_mnemoic.unwrap());
    }

    constexpr IResult<uint16_t> query_major(const Mnemonic mnemoic) const {
        return details::query_tmp<uint16_t>(line, mnemoic.to_letter(), 
        [](const StringView str) -> Result<uint16_t, Error>{
            const auto res = (strconv2::FstrDump::parse(str.substr(1).unwrap()));
            if(res.is_err()) return Err(res.unwrap_err());
            const auto dump = res.unwrap();
            if(dump.digit_part > std::numeric_limits<uint16_t>::max())
                return Err(GcodeParseError::MajorNumberOverflow);
            return Ok(uint16_t(dump.digit_part));
        });
    };

    constexpr IResult<uint16_t> query_minor(const Mnemonic mnemoic) const {
        return details::query_tmp<uint16_t>(line, mnemoic.to_letter(), 
        [](const StringView str) -> Result<uint16_t, Error>{
            const auto res = (strconv2::FstrDump::parse(str.substr(1).unwrap()));
            if(res.is_err()) return Err(res.unwrap_err());
            const auto dump = res.unwrap();
            if(dump.num_frac_digits == 0) return Err(GcodeParseError::NoMinorNumber);
            if(dump.frac_part > std::numeric_limits<uint16_t>::max())
                return Err(GcodeParseError::MinorNumberOverflow);
            return Ok(uint16_t(dump.frac_part));
        });
    };

    constexpr IResult<iq16> query_arg_value(const char letter) const {
        return details::query_tmp<iq16>(line, letter, [](const StringView str) -> IResult<iq16>{
            const auto res = (strconv2::defmt_from_str<iq16>(str.substr(1).unwrap()));
            if(res.is_err()) return Err(res.unwrap_err());
            return Ok(res.unwrap());
        });
    }
};

struct [[nodiscard]] GcodeScriptLine final{
    StringView str;
    static constexpr char SPLIT_CHAR = ';';

    [[nodiscard]] constexpr Option<StringView> comment() const {
        const auto comment_begin_it = std::find(str.begin(), str.end(), SPLIT_CHAR);
        if(comment_begin_it == str.end()) return None;
        return Some(StringView(comment_begin_it, str.end()));
    }


    [[nodiscard]] constexpr Option<StringView> code() const {
        if(str.begin() == str.end()) return None;
        const auto comment_begin_it = std::find(str.begin(), str.end(), SPLIT_CHAR);
        if(comment_begin_it == str.end()) return None;
        if(comment_begin_it == str.begin()) return None;
        auto code_str = StringView(str.begin(), comment_begin_it);
        code_str = code_str.trim();
        if(code_str.length() == 0) return None;
        return Some(code_str);
    }

};

}
