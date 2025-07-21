#pragma once

#include "core/string/string_view.hpp"
#include "core/utils/result.hpp"
#include "core/string/utils/multiline_split.hpp"
#include "core/string/utils/strconv2.hpp"

namespace ymd::gcode{


enum class GcodeParseError:uint8_t{
    NoLetterFounded,
    NoStringSegmentFounded,
    NoMajorNumber,
    NoMinorNumber,
    MajorNumberOverflow,
    MinorNumberOverflow,

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

    template<char letter>
    static consteval Mnemonic from_letter(){
        return from_letter(letter).unwrap();
    }

    constexpr Mnemonic(const Kind kind):
        kind_(kind){;}

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
    Kind kind_;
public:
    using enum Kind;

    friend OutputStream & operator<<(OutputStream & os, const Mnemonic self){
        return os << self.kind();
    }
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

    constexpr IResult<Mnemonic> query_mnemonic() const {
        if(line_.length() < 1) 
            return Err(GcodeParseError::NoMnemonicFounded);
        return ({
            auto may_mnemoic = Mnemonic::from_letter(line_[0]);
            if(may_mnemoic.is_none()) 
                return Err(GcodeParseError::InvalidMnemonic);
            Ok(may_mnemoic.unwrap());
        });
    }

    constexpr IResult<uint16_t> query_major(Mnemonic mnemoic) const {
        return query_tmp<uint16_t>(mnemoic.to_letter(), 
        [](const StringView str) -> Result<uint16_t, Error>{
            const auto res = (strconv2::FstrDump::from_str(str.substr(1)));
            if(res.is_err()) return Err(res.unwrap_err());
            const auto dump = res.unwrap();
            if(dump.digit_part > std::numeric_limits<uint16_t>::max())
                return Err(GcodeParseError::MajorNumberOverflow);
            return Ok(uint16_t(dump.digit_part));
        });
    };

    constexpr IResult<uint16_t> query_minor(Mnemonic mnemoic) const {
        return query_tmp<uint16_t>(mnemoic.to_letter(), 
        [](const StringView str) -> Result<uint16_t, Error>{
            const auto res = (strconv2::FstrDump::from_str(str.substr(1)));
            if(res.is_err()) return Err(res.unwrap_err());
            const auto dump = res.unwrap();
            if(dump.scale == 0) return Err(GcodeParseError::NoMinorNumber);
            if(dump.frac_part > std::numeric_limits<uint16_t>::max())
                return Err(GcodeParseError::MinorNumberOverflow);
            return Ok(uint16_t(dump.frac_part));
        });
    };

    constexpr IResult<q16> query_arg_value(const char letter) const {
        return query_tmp<q16>(letter, [](const StringView str) -> IResult<q16>{
            const auto res = (strconv2::str_to_iq<16>(str.substr(1)));
            if(res.is_err()) return Err(res.unwrap_err());
            return Ok(res.unwrap());
        });
    }

private:
    StringView line_;

    constexpr IResult<StringView> query_value_str(const char letter) const {
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
    constexpr IResult<T> query_tmp(const char letter, Fn && fn) const {
        const auto either_str = query_value_str(letter);
        if(either_str.is_err()) return Err(either_str.unwrap_err());
        const auto str = either_str.unwrap();
        return std::forward<Fn>(fn)(str);
    }


};


namespace test{

static inline void static_test(){
    // 测试助记符解析
    static_assert(GcodeLine("G28").query_mnemonic().unwrap() == Mnemonic::General);
    static_assert(GcodeLine("M106").query_mnemonic().unwrap() == Mnemonic::Miscellaneous);
    // static_assert(GcodeLine("X10").query_mnemonic().unwrap().to_letter() == ' '); // 假设X不是合法助记符

    // 测试主/次编号
    static_assert(GcodeLine("G28").query_major(Mnemonic::General).unwrap() == 28);
    static_assert(GcodeLine("G28.3").query_minor(Mnemonic::General).unwrap() == 3);
    static_assert(GcodeLine("G1").query_minor(Mnemonic::General).is_err()); // 无次编号

}


}
}
