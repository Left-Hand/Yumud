#pragma once

#include "core/string/conv/strconv2.hpp"
#include "core/string/utils/split_iter.hpp"
#include "core/utils/result.hpp"

namespace ymd::gcode{




enum class GcodeParseError:uint8_t{
    NoLetterFounded,
    NoStringSegmentFounded,
    NoMajorNumber,
    NoMinorNumber,
    MajorNumberOverflow,
    MinorNumberOverflow,
    UnknownHeadLetter,
    NoMnemonicFounded,
    InvalidMnemonic,
    InvalidArgumentLetter
};

DEF_DERIVE_DEBUG(GcodeParseError)

DEF_ERROR_WITH_KINDS(Error, GcodeParseError, strconv2::DestringError)

template<typename T = void>
using IResult = Result<T, Error>;   

struct [[nodiscard]] Mnemonic final{
    enum class [[nodiscard]] Kind:uint8_t {
        General = 0,
        Miscellaneous = 1,
        ProgramNumber = 2,
        ToolChange = 3,
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


struct [[nodiscard]] GcodeArg final{
    char letter;
    iq16 value;

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

    friend OutputStream & operator << (OutputStream & os, const GcodeArg & self){
        return os << os.brackets<'('>() 
            << self.letter << os.splitter()
            << self.value << os.brackets<')'>();
    }
};


struct [[nodiscard]] GcodeArgsIter final{
    constexpr explicit GcodeArgsIter(StringView line)
        : arg_str_iter_(line, ' ') {}

    [[nodiscard]] constexpr bool has_next() const {
        // No more arguments -> return None
        return arg_str_iter_.has_next();
    }

    constexpr IResult<GcodeArg> next() {
        while (arg_str_iter_.has_next()) {
            const auto token_str = arg_str_iter_.next();
            if (token_str.length() == 0) continue; // Skip empty tokens (e.g., multiple spaces)

            const char letter = token_str[0];
            // if (Mnemonic::is_letter_valid(letter))
            //     continue;
            if (!GcodeArg::is_letter_valid(letter)) {
                // Invalid letter (e.g., "@100") -> return Error
                return Err(GcodeParseError::InvalidArgumentLetter);
            }

            // Parse value (skip letter)
            const auto value_str = token_str.substr(1).unwrap();
            const auto res = strconv2::defmt_from_str<iq16>(value_str);
            if (res.is_err()) {
                // Value parsing failed (e.g., "X1.2.3") -> return Error
                return Err(res.unwrap_err());
            }

            // Valid argument -> return Ok(GcodeArg)
            return Ok(GcodeArg{
                .letter = letter,
                .value = res.unwrap()
            });
        }

        __builtin_unreachable();
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
        if(line.length() < 1) 
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



}
