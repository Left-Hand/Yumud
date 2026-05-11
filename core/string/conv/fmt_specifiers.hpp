#pragma once

#include <cstdint>
#include <cstddef>

#include "core/utils/Option.hpp"

namespace ymd::fmt_specifiers{

// Format specifiers matching std::println/std::format syntax
// Grammar: [[fill]align][sign][#][0][width][,][.precision][type]
//
// Supported format specifiers:
// - fill: any character (default: space)
// - align: '<' (left), '>' (right), '^' (center), '=' (sign-aware, numeric only)
// - sign: '+' (always), '-' (negative only), ' ' (space for positive)
// - '#': alternate form (0x/0o/0b prefix, trailing decimal point)
// - '0': zero padding (numeric types only)
// - width: minimum field width
// - ',': thousands separator (integers only)
// - precision: decimal places (floats) or max length (strings)
// - type: 'b'/'o'/'d'/'x'/'X' (int), 'f'/'e'/'g'/'a' (float), 's' (string), 'c' (char), 'p' (pointer)

struct [[nodiscard]] Alignment final{
    enum class [[nodiscard]] Kind:uint8_t{
        Default = 0,
        Left,
        Right,
        Center,
        SignAware,
    };

    using enum Kind;

    Kind kind;

    static constexpr Option<Alignment> try_from_char(const char c){
        Kind kind;
        switch(c){
            default: return None;
            case '<':
                kind = Kind::Left;
                break;
            case '>':
                kind = Kind::Right;
                break;
            case '^':
                kind = Kind::Center;
                break;
            case '=':
                kind = Kind::SignAware;
                break;
        }

        return Some(Alignment{kind});
    }
};

struct [[nodiscard]] Signment final{
    enum class [[nodiscard]] Kind:uint8_t{
        Default = 0,
        Always,       // '+'
        NegativeOnly, // '-'
        Space,        // ' '
    };

    using enum Kind;

    Kind kind;

    static constexpr Option<Signment> try_from_char(const char c){
        Kind kind;
        switch(c){
            default: return None;
            case '+':
                kind = Kind::Always;
                break;
            case '-':
                kind = Kind::NegativeOnly;
                break;
            case ' ':
                kind = Kind::Space;
                break;
        }

        return Some(Signment{kind});
    }
};

struct [[nodiscard]] IntRadix final{
    enum class [[nodiscard]] Kind:uint8_t{
        Default = 0, // decimal
        Binary,      // 'b'
        Octal,       // 'o'
        Decimal,     // 'd'
        HexLower,    // 'x'
        HexUpper,    // 'X'
        Char,        // 'c'
    };

    using enum Kind;

    Kind kind;

    static constexpr Option<IntRadix> try_from_char(const char c){
        Kind kind;
        switch(c){
            default: return None;
            case 'b':
                kind = Kind::Binary;
                break;
            case 'o':
                kind = Kind::Octal;
                break;
            case 'd':
                kind = Kind::Decimal;
                break;
            case 'x':
                kind = Kind::HexLower;
                break;
            case 'X':
                kind = Kind::HexUpper;
                break;
            case 'c':
                kind = Kind::Char;
                break;
        }

        return Some(IntRadix{kind});
    }
};

struct [[nodiscard]] FloatingType final{
    enum class [[nodiscard]] Kind:uint8_t{
        Default = 0, // fixed
        Fixed,       // 'f'
        Scientific,  // 'e'
        Adaptive,    // 'g'
        Hexadecimal, // 'a'
    };

    using enum Kind;

    Kind kind;

    static constexpr Option<FloatingType> try_from_char(const char c){
        Kind kind;
        switch(c){
            default: return None;
            case 'f':
                kind = Kind::Fixed;
                break;
            case 'e':
                kind = Kind::Scientific;
                break;
            case 'g':
                kind = Kind::Adaptive;
                break;
            case 'a':
                kind = Kind::Hexadecimal;
                break;
        }

        return Some(FloatingType{kind});
    }
};

struct [[nodiscard]] BoolType final{
    enum class [[nodiscard]] Kind:uint8_t{
        Numeric = 0, // '0'/'1'
        Text,        // 's' -> "true"/"false"
    };

    using enum Kind;

    Kind kind;

    static constexpr Option<BoolType> try_from_char(const char c){
        if(c == 's') return Some(BoolType{Kind::Text});
        return None;
    }
};


struct [[nodiscard]] alignas(4) CommonSpecifier final {

    Alignment::Kind align_kind:3;

    Signment::Kind sign_kind:2;

    // '0' flag: zero padding (only valid with numeric types)
    uint8_t zero_padding_flag:1;

    // ',' flag: thousands separator for integers
    uint8_t has_thousands_separator:1;

    // '#' flag: alternate form (0x/0o/0b prefix for ints, trailing decimal point for floats)
    uint8_t explicit_alternate:1;

    
    uint8_t fill;
    
    // minimum field width, 0 = no padding
    uint16_t width;


    static constexpr CommonSpecifier from_default() {
        CommonSpecifier self;
        self.align_kind = Alignment::Kind::Default;
        self.sign_kind = Signment::Kind::Default;

        self.zero_padding_flag = false;
        self.has_thousands_separator = false;
        self.explicit_alternate = false;
        self.fill = ' ';
        self.width = 0;
        return self;
    }

    [[nodiscard]] constexpr const char* parse_own_specified(std::string_view str) {
        // - **填充字符与对齐方式（[fill][align]）**  
        // 若字符串至少包含两个字符，则检查第二个字符是否为有效的对齐符号（`<`、`>`、`^` 或 `=`）。  
        // 如果是，则将第一个字符视为填充字符（fill），第二个字符决定对齐方式，并消耗这两个字符。  

        // - **单独的对齐符号（align）**  
        // 若尚未设置对齐方式（即仍为默认值），且当前仍有剩余字符，则尝试将当前字符作为对齐符号解析。  
        // 若成功，则设置对齐方式，并消耗该字符。

        // - **符号显示策略（sign）**  
        // 若当前位置存在字符且为 `+`、`-` 或空格 `' '`，则将其解释为符号显示规则：  
        // - `+` 表示总是显示正负号，  
        // - `-` 表示仅负数显示符号，  
        // - 空格表示正数前加空格、负数显示负号。  
        // 解析后消耗该字符。

        // - **替代形式标志（#）**  
        // 若当前字符是 `#`，则启用“替代形式”：例如整数输出时添加 `0x`、`0o`、`0b` 前缀，浮点数保留小数点等。  
        // 解析后消耗该字符。

        // - **零填充标志（0）**  
        // 若当前字符是 `0`，且尚未显式指定对齐方式（即对齐仍为默认），则启用零填充（zero padding）。  
        // 注意：一旦用户已写出对齐符号（如 `<`），则 `0` 不再表示零填充，而会被后续当作宽度的一部分处理。  
        // 若条件满足，则消耗该字符。

        // - **最小字段宽度（width）**  
        // 从当前位置开始，连续读取十进制数字字符，将其组合成一个整数，作为最小输出宽度。  
        // 每读取一个数字字符，游标前移一位，直到遇到非数字字符或字符串结束。

        // - **千位分隔符标志（,）**  
        // 若当前字符是 `,`，则启用千位分隔符（仅适用于整数类型）。  
        // 解析后消耗该字符。

        auto& self = *this;
        const char* cursor = str.data();
        const char* end = str.end();

        // Step 1: Parse [[fill]align]
        if (cursor + 1 < end) {
            Alignment::try_from_char(*(cursor + 1)).inspect([&](auto align) {
                self.fill = *cursor;
                self.align_kind = align.kind;
                cursor += 2;
            });
        }

        // If no fill+align found, check for align-only
        if (self.align_kind == Alignment::Kind::Default and cursor < end) {
            Alignment::try_from_char(*cursor).inspect([&](auto align) {
                self.align_kind = align.kind;
                cursor += 1;
            });
        }

        // Step 2: Parse [sign]
        if (cursor < end) {
            Signment::try_from_char(*cursor).inspect([&](auto slgn) {
                self.sign_kind = slgn.kind;
                cursor += 1;
            });
        }

        // Step 3: Parse [#] flag
        if (cursor < end and *cursor == '#') {
            self.explicit_alternate = true;
            cursor += 1;
        }

        // Step 4: Parse [0] flag (only valid without explicit align)
        if (cursor < end and *cursor == '0' and self.align_kind == Alignment::Kind::Default) {
            self.zero_padding_flag = true;
            cursor += 1;
        }

        // Step 5: Parse [width]
        while (cursor < end and *cursor >= '0' and *cursor <= '9') {
            self.width = self.width * 10 + (*cursor - '0');
            cursor += 1;
        }

        // Step 6: Parse [,] flag
        if (cursor < end and *cursor == ',') {
            self.has_thousands_separator = true;
            cursor += 1;
        }

        return cursor;
    }

    static constexpr CommonSpecifier from_str(std::string_view str){
        auto self = from_default();
        [[maybe_unused]] auto end = self.parse_own_specified(str);
        return self;
    }
};

static_assert(sizeof(CommonSpecifier) == 4);
static_assert(CommonSpecifier::from_str("<").align_kind == Alignment::Kind::Left);
static_assert(CommonSpecifier::from_str(">").align_kind == Alignment::Kind::Right);

struct [[nodiscard]] IntSpecifier final {
    CommonSpecifier super;

    IntRadix::Kind radix_kind;

    static constexpr IntSpecifier from_default() {
        return {
            .super = CommonSpecifier::from_default(),
            .radix_kind = IntRadix::Kind::Default
        };
    }

    [[nodiscard]] constexpr const char * parse_own_specified(std::string_view str){
        auto & self = *this;
        auto begin = str.data();
        auto end = str.data() + str.size();

        // Parse [radix_kind]
        if(begin < end){
            IntRadix::try_from_char(*begin).inspect([&](auto int_radix){
                self.radix_kind = int_radix.kind;
                begin++;
            });
        }

        return begin;
    }

    static constexpr IntSpecifier from_str(std::string_view str){
        auto self = from_default();
        auto begin = str.data();
        auto end = str.data() + str.size();

        begin = self.super.parse_own_specified({begin, end});
        begin = self.parse_own_specified({begin, end});

        return self;
    }
};

struct [[nodiscard]] FloatingSpecifier final {
    CommonSpecifier super;

    FloatingType::Kind type;

    // decimal places (f/e) or significant digits (g)
    // 255 = default (6)
    uint8_t precision;

    static constexpr FloatingSpecifier from_default() {
        return {
            .super = CommonSpecifier::from_default(),
            .type = FloatingType::Kind::Default,
            .precision = 255
        };
    }

    [[nodiscard]] constexpr const char * parse_own_specified(std::string_view str){
        auto & self = *this;
        auto begin = str.data();
        auto end = str.data() + str.size();

        // Parse [.precision]
        if(begin < end and *begin == '.'){
            begin++;
            uint8_t prec = 0;
            while(begin < end and *begin >= '0' and *begin <= '9'){
                prec = prec * 10 + (*begin - '0');
                begin++;
            }
            self.precision = prec;
        }

        // Parse [type]
        if(begin < end){
            FloatingType::try_from_char(*begin).inspect([&](auto float_type){
                self.type = float_type.kind;
                begin++;
            });
        }

        return begin;
    }

    static constexpr FloatingSpecifier from_str(std::string_view str){
        auto self = from_default();
        auto begin = str.data();
        auto end = str.data() + str.size();

        begin = self.super.parse_own_specified({begin, end});
        begin = self.parse_own_specified({begin, end});

        return self;
    }
};

struct [[nodiscard]] StringSpecifier final {
    CommonSpecifier super;

    // maximum length for string output
    // 255 = no limit
    uint8_t precision;

    static constexpr StringSpecifier from_default() {
        return {
            .super = CommonSpecifier::from_default(),
            .precision = 255
        };
    }

    [[nodiscard]] constexpr const char * parse_own_specified(std::string_view str){
        auto & self = *this;
        auto begin = str.data();
        auto end = str.data() + str.size();

        // Parse [.precision]
        if(begin < end and *begin == '.'){
            begin++;
            uint8_t prec = 0;
            while(begin < end and *begin >= '0' and *begin <= '9'){
                prec = prec * 10 + (*begin - '0');
                begin++;
            }
            self.precision = prec;
        }

        return begin;
    }

    static constexpr StringSpecifier from_str(std::string_view str){
        auto self = from_default();
        auto begin = str.data();
        auto end = str.data() + str.size();

        begin = self.super.parse_own_specified({begin, end});
        begin = self.parse_own_specified({begin, end});

        return self;
    }
};

struct [[nodiscard]] CharSpecifier final {
    CommonSpecifier super;

    static constexpr CharSpecifier from_default() {
        return {
            .super = CommonSpecifier::from_default()
        };
    }

    [[nodiscard]] constexpr const char * parse_own_specified(std::string_view str){
        // CharSpecifier has no additional specifiers beyond CommonSpecifier
        return str.data();
    }

    static constexpr CharSpecifier from_str(std::string_view str){
        auto self = from_default();
        auto begin = str.data();
        auto end = str.data() + str.size();

        begin = self.super.parse_own_specified({begin, end});
        begin = self.parse_own_specified({begin, end});

        return self;
    }
};

struct [[nodiscard]] PointerSpecifier final {
    CommonSpecifier super;

    // pointer always formatted as hexadecimal
    // only supports fill/align/width from CommonSpecifier

    static constexpr PointerSpecifier from_default() {
        return {
            .super = CommonSpecifier::from_default()
        };
    }

    [[nodiscard]] constexpr const char * parse_own_specified(std::string_view str){
        // PointerSpecifier has no additional specifiers beyond CommonSpecifier
        return str.data();
    }

    static constexpr PointerSpecifier from_str(std::string_view str){
        auto self = from_default();
        auto begin = str.data();
        auto end = str.data() + str.size();

        begin = self.super.parse_own_specified({begin, end});
        begin = self.parse_own_specified({begin, end});

        return self;
    }
};

struct [[nodiscard]] BoolSpecifier final {
    CommonSpecifier super;

    BoolType::Kind type;

    static constexpr BoolSpecifier from_default() {
        return {
            .super = CommonSpecifier::from_default(),
            .type = BoolType::Kind::Numeric
        };
    }

    [[nodiscard]] constexpr const char * parse_own_specified(std::string_view str){
        auto & self = *this;
        auto begin = str.data();
        auto end = str.data() + str.size();

        // Parse [type] ('s' for text format)
        if(begin < end){
            BoolType::try_from_char(*begin).inspect([&](auto bool_type){
                self.type = bool_type.kind;
                begin++;
            });
        }

        return begin;
    }

    static constexpr BoolSpecifier from_str(std::string_view str){
        auto self = from_default();
        auto begin = str.data();
        auto end = str.data() + str.size();

        begin = self.super.parse_own_specified({begin, end});
        begin = self.parse_own_specified({begin, end});

        return self;
    }
};

namespace{
// Tests - CommonSpecifier
static_assert(CommonSpecifier::from_str("<").align_kind == Alignment::Kind::Left);
static_assert(CommonSpecifier::from_str(">").align_kind == Alignment::Kind::Right);
static_assert(CommonSpecifier::from_str("^").align_kind == Alignment::Kind::Center);
static_assert(CommonSpecifier::from_str("=").align_kind == Alignment::Kind::SignAware);
static_assert(CommonSpecifier::from_str("+").sign_kind == Signment::Kind::Always);
static_assert(CommonSpecifier::from_str("-").sign_kind == Signment::Kind::NegativeOnly);
static_assert(CommonSpecifier::from_str(" ").sign_kind == Signment::Kind::Space);
static_assert(CommonSpecifier::from_str("#").explicit_alternate);
static_assert(CommonSpecifier::from_str("010").zero_padding_flag and CommonSpecifier::from_str("010").width == 10);
static_assert(CommonSpecifier::from_str("100").width == 100);
static_assert(CommonSpecifier::from_str(",").has_thousands_separator);
static_assert(CommonSpecifier::from_str("*<").fill == '*' and CommonSpecifier::from_str("*<").align_kind == Alignment::Kind::Left);
static_assert(CommonSpecifier::from_str("*>").fill == '*' and CommonSpecifier::from_str("*>").align_kind == Alignment::Kind::Right);
static_assert(CommonSpecifier::from_str("+010,").sign_kind == Signment::Kind::Always and
            CommonSpecifier::from_str("+010,").zero_padding_flag and
            CommonSpecifier::from_str("+010,").width == 10 and
            CommonSpecifier::from_str("+010,").has_thousands_separator);

// Tests - IntSpecifier
static_assert(IntSpecifier::from_str("x").radix_kind == IntRadix::Kind::HexLower);
static_assert(IntSpecifier::from_str("X").radix_kind == IntRadix::Kind::HexUpper);
static_assert(IntSpecifier::from_str("b").radix_kind == IntRadix::Kind::Binary);
static_assert(IntSpecifier::from_str("o").radix_kind == IntRadix::Kind::Octal);
static_assert(IntSpecifier::from_str("d").radix_kind == IntRadix::Kind::Decimal);
static_assert(IntSpecifier::from_str("c").radix_kind == IntRadix::Kind::Char);
static_assert(IntSpecifier::from_str("#x").super.explicit_alternate 
    and IntSpecifier::from_str("#x").radix_kind == IntRadix::Kind::HexLower);
static_assert(IntSpecifier::from_str("#b").super.explicit_alternate 
    and IntSpecifier::from_str("#b").radix_kind == IntRadix::Kind::Binary);
static_assert(IntSpecifier::from_str(">+010x").super.align_kind == Alignment::Kind::Right);
static_assert(IntSpecifier::from_str(">+010x").super.sign_kind == Signment::Kind::Always);
static_assert(!IntSpecifier::from_str(">+010x").super.zero_padding_flag);  // '0' is part of width when align is explicit
static_assert(IntSpecifier::from_str(">+010x").super.width == 10);
static_assert(IntSpecifier::from_str("+010x").super.zero_padding_flag);     // '0' is padding when no explicit align
static_assert(IntSpecifier::from_str("+010x").super.width == 10);
static_assert(IntSpecifier::from_str("010x").super.width == 10 
    and IntSpecifier::from_str("010x").radix_kind == IntRadix::Kind::HexLower);
static_assert(IntSpecifier::from_str("+,d").super.sign_kind == Signment::Kind::Always 
    and IntSpecifier::from_str("+,d").super.has_thousands_separator);
static_assert(IntSpecifier::from_str("*>10,x").super.fill == '*' 
    and IntSpecifier::from_str("*>10,x").super.width == 10);

// Tests - FloatingSpecifier
static_assert(FloatingSpecifier::from_str("f").type == FloatingType::Kind::Fixed);
static_assert(FloatingSpecifier::from_str("e").type == FloatingType::Kind::Scientific);
static_assert(FloatingSpecifier::from_str("g").type == FloatingType::Kind::Adaptive);
static_assert(FloatingSpecifier::from_str("a").type == FloatingType::Kind::Hexadecimal);
static_assert(FloatingSpecifier::from_str(".2f").precision == 2 
    and FloatingSpecifier::from_str(".2f").type == FloatingType::Kind::Fixed);
static_assert(FloatingSpecifier::from_str(".10e").precision == 10 
    and FloatingSpecifier::from_str(".10e").type == FloatingType::Kind::Scientific);
static_assert(FloatingSpecifier::from_str("#.2f").super.explicit_alternate 
    and FloatingSpecifier::from_str("#.2f").precision == 2);
static_assert(FloatingSpecifier::from_str("+.3g").super.sign_kind == Signment::Kind::Always 
    and FloatingSpecifier::from_str("+.3g").precision == 3);
static_assert(FloatingSpecifier::from_str(">10.5f").super.width == 10 
    and FloatingSpecifier::from_str(">10.5f").precision == 5);
static_assert(FloatingSpecifier::from_str("010.2f").super.zero_padding_flag 
    and FloatingSpecifier::from_str("010.2f").precision == 2);


// Tests - StringSpecifier
static_assert(StringSpecifier::from_str(".10").precision == 10);
static_assert(StringSpecifier::from_str(".5s").precision == 5);
static_assert(StringSpecifier::from_str(">10.5").super.width == 10 
    and StringSpecifier::from_str(">10.5").precision == 5);
static_assert(StringSpecifier::from_str("<20.15").super.align_kind == Alignment::Kind::Left 
    and StringSpecifier::from_str("<20.15").super.width == 20);
static_assert(StringSpecifier::from_str("*^30.20").super.fill == '*' 
    and StringSpecifier::from_str("*^30.20").super.width == 30);
static_assert(StringSpecifier::from_str("10").super.width == 10);

// Tests - BoolSpecifier
static_assert(BoolSpecifier::from_str("s").type == BoolType::Kind::Text);
static_assert(BoolSpecifier::from_str("").type == BoolType::Kind::Numeric);
static_assert(BoolSpecifier::from_str(">10s").super.width == 10 
    and BoolSpecifier::from_str(">10s").type == BoolType::Kind::Text);
static_assert(BoolSpecifier::from_str("<15").super.width == 15 
    and BoolSpecifier::from_str("<15").type == BoolType::Kind::Numeric);

// Tests - CharSpecifier
static_assert(CharSpecifier::from_str(">10").super.width == 10 
    and CharSpecifier::from_str(">10").super.align_kind == Alignment::Kind::Right);
static_assert(CharSpecifier::from_str("^5").super.width == 5 
    and CharSpecifier::from_str("^5").super.align_kind == Alignment::Kind::Center);
static_assert(CharSpecifier::from_str("*<8").super.fill == '*' 
    and CharSpecifier::from_str("*<8").super.width == 8);

// Tests - PointerSpecifier
static_assert(PointerSpecifier::from_str(">20").super.width == 20 
    and PointerSpecifier::from_str(">20").super.align_kind == Alignment::Kind::Right);
static_assert(PointerSpecifier::from_str("010").super.zero_padding_flag 
    and PointerSpecifier::from_str("010").super.width == 10);
static_assert(PointerSpecifier::from_str("*^16").super.fill == '*' 
    and PointerSpecifier::from_str("*^16").super.width == 16);

}


}


