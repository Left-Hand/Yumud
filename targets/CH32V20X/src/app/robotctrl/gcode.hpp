#pragma once

#include "core/string/string_view.hpp"
#include "core/utils/result.hpp"
#include "core/string/utils/multiline_split.hpp"
#include "core/string/utils/strconv2.hpp"

namespace ymd::gcode{


enum class GcodeParseError:uint8_t{
    NoLetterFounded,
    NoStringSegmentFounded,
    GcodeHasNoMajorNumber,
    GcodeHasNoMinorNumber,
    NoMnemonicFounded,
    InvalidMnemonic,
};

DEF_DERIVE_DEBUG(GcodeParseError)

DEF_ERROR_WITH_KINDS(Error, GcodeParseError, strconv2::DestringError)

template<typename T = void>
using IResult = Result<T, Error>;   

struct Mnemonic final{
    enum class Kind:uint8_t {
        General = 0,
        Miscellaneous = 1,
        ProgramNumber = 2,
        ToolChange = 3,
    };

    DEF_FRIEND_DERIVE_DEBUG(Kind);

    static constexpr Option<Mnemonic> from_letter(const char letter){
        switch(letter){
            case 'G':
                return Some(Mnemonic{Kind::General});
            case 'M':
                return Some(Mnemonic{Kind::Miscellaneous});
            case 'T':
                return Some(Mnemonic{Kind::ToolChange});
            case 'N':
                return Some(Mnemonic{Kind::ProgramNumber});
            default:
                return None;
        }
    }

    constexpr bool operator==(const Mnemonic & other) const{
        return kind_ == other.kind_;
    }

    constexpr bool operator==(const Kind kind) const {
        return kind_ == kind;
    }

    constexpr Kind kind() const {
        return kind_;
    }

    constexpr char to_letter() const {
        switch(kind_){
            case Kind::General:
                return 'G';
            case Kind::Miscellaneous:
                return 'M';
            case Kind::ProgramNumber:
                return 'P';
            case Kind::ToolChange:
                return 'T';
            default:
                __builtin_unreachable();
        }
    }

    constexpr Mnemonic(const Mnemonic & other) = default; 
    constexpr Mnemonic(Mnemonic && other) = default; 
private:
    explicit constexpr Mnemonic(Kind kind):kind_(kind){}
    Kind kind_;
public:
    using enum Kind;

    friend OutputStream & operator<<(OutputStream & os, const Mnemonic self){
        return os << self.kind();
    }
};
struct LineText{
    explicit LineText(const StringView text):text_(text){}
    
    constexpr StringView text(){
        return text_;
    };
private:
    StringView text_;

};

struct MultiLineText{
    Option<StringView> get_line(const size_t line_nth) const {
        //TODO
        return None;
    }
private:
};

struct SourceLocation{
    uint8_t start;    // Starting column
    uint8_t end;      // Ending column  
    uint16_t line;    // Line number
    
    friend OutputStream& operator<<(OutputStream& os, const SourceLocation self) {
        return os << os.brackets<'('>() 
            << self.start << os.splitter()
            << self.end << os.splitter()
            << self.line
            << os.brackets<')'>();
    }
};

struct Word{
    char letter;
    float value;
    SourceLocation source;
};

struct GcodeArg{
    char letter;
    q16 value;
};

struct GcodeLine{
    constexpr explicit GcodeLine(StringView line):
        line_(line){;}

    IResult<Mnemonic> query_mnemonic(const char letter) const {
        return query_tmp<Mnemonic>(letter, 
        [](const StringView str) -> Result<Mnemonic, Error>{
            if(str.length() < 1) 
                return Err(GcodeParseError::NoMnemonicFounded);
            return ({
                auto may_mnemoic = Mnemonic::from_letter(str[0]);
                if(may_mnemoic.is_none()) 
                    return Err(GcodeParseError::InvalidMnemonic);
                Ok(may_mnemoic.unwrap());
            });
        });
    }

    IResult<uint8_t> query_major(Mnemonic mnemoic) const {
        return query_tmp<uint8_t>(mnemoic.to_letter(), 
        [](const StringView str) -> Result<uint8_t, Error>{
            const auto res = (strconv2::FstrDump::from_str(str.substr(1)));
            if(res.is_err()) return Err(res.unwrap_err());
            const auto dump = res.unwrap();
            return Ok(uint8_t(dump.digit_part));
        });
    };

    IResult<uint8_t> query_minor(Mnemonic mnemoic) const {
        return query_tmp<uint8_t>(mnemoic.to_letter(), 
        [](const StringView str) -> Result<uint8_t, Error>{
            const auto res = (strconv2::FstrDump::from_str(str.substr(1)));
            if(res.is_err()) return Err(res.unwrap_err());
            const auto dump = res.unwrap();
            if(dump.scale == 0) return Err(GcodeParseError::GcodeHasNoMinorNumber);
            return Ok(uint8_t(dump.frac_part));
        });
    };

    IResult<q16> query_arg_value(const char letter) const {
        return query_tmp<q16>(letter, [](const StringView str) -> IResult<q16>{
            const auto res = (strconv2::str_to_iq<16>(str));
            if(res.is_err()) return Err(res.unwrap_err());
            return Ok(res.unwrap());
        });
    }

private:

    IResult<StringView> query_value_str(const char letter) const {
        auto value_position = ({
            const auto may_index = line_.find(letter);
            if(may_index.is_none()) 
                return Err(GcodeParseError::NoLetterFounded);
            may_index.unwrap();
        });

        auto iter = strconv2::StringSplitIter(
            line_.substr(value_position), ' '
        );

        if(not iter.has_next()) 
            return Err(GcodeParseError::NoStringSegmentFounded);
        return Ok(iter.next());
    }

    template<typename T, typename Fn>
    IResult<T> query_tmp(const char letter, Fn && fn) const {
        const auto either_str = query_value_str(letter);
        if(either_str.is_err()) return Err(either_str.unwrap_err());
        const auto str = either_str.unwrap();
        return std::forward<Fn>(fn)(str);
    }
    StringView line_;
};

struct GCodeCommandSpawner{

};
}
