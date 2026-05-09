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

struct [[nodiscard]] Sign final{
    enum class [[nodiscard]] Kind:uint8_t{
        Default = 0,
        Always,       // '+'
        NegativeOnly, // '-'
        Space,        // ' '
    };

    using enum Kind;

    Kind kind;

    static constexpr Option<Sign> try_from_char(const char c){
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

        return Some(Sign{kind});
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


struct [[nodiscard]] CommonSpecifier final {
    uint8_t fill;

    // minimum field width, 0 = no padding
    uint8_t width;

    Alignment::Kind align_kind;

    Sign::Kind sign_kind;

    // '0' flag: zero padding (only valid with numeric types)
    uint8_t zero_padding:1;

    // ',' flag: thousands separator for integers
    uint8_t thousands_separator:1;

    // '#' flag: alternate form (0x/0o/0b prefix for ints, trailing decimal point for floats)
    uint8_t explicit_alternate:1;

    static constexpr CommonSpecifier from_default() {
        return {
            .fill = ' ',
            .width = 0,
            .align_kind = Alignment::Kind::Default,
            .sign_kind = Sign::Kind::Default,
            .zero_padding = false,
            .thousands_separator = false,
            .explicit_alternate = false
        };
    }

    [[nodiscard]] constexpr const char * parse_own_specified(std::string_view str){
        auto & self = *this;
        size_t i = 0;

        // Step 1: Parse [[fill]align]
        if(i + 1 < str.size()){
            Alignment::try_from_char(str[i + 1]).inspect([&](auto align){
                self.fill = str[i];
                self.align_kind = align.kind;
                i += 2;
            });
        }

        // If no fill+align found, check for align-only
        if(self.align_kind == Alignment::Kind::Default && i < str.size()){
            Alignment::try_from_char(str[i]).inspect([&](auto align){
                self.align_kind = align.kind;
                i += 1;
            });
        }

        // Step 2: Parse [sign]
        if(i < str.size()){
            Sign::try_from_char(str[i]).inspect([&](auto slgn){
                self.sign_kind = slgn.kind;
                i += 1;
            });
        }

        // Step 3: Parse [#] flag
        if(i < str.size() && str[i] == '#'){
            self.explicit_alternate = true;
            i += 1;
        }

        // Step 4: Parse [0] flag (only valid without explicit align)
        if(i < str.size() && str[i] == '0' && self.align_kind == Alignment::Kind::Default){
            self.zero_padding = true;
            i += 1;
        }

        // Step 5: Parse [width]
        while(i < str.size() && str[i] >= '0' && str[i] <= '9'){
            self.width = self.width * 10 + (str[i] - '0');
            i += 1;
        }

        // Step 6: Parse [,] flag
        if(i < str.size() && str[i] == ','){
            self.thousands_separator = true;
            i += 1;
        }

        return str.data() + i;
    }

    static constexpr CommonSpecifier from_str(std::string_view str){
        auto self = from_default();
        [[maybe_unused]] auto end = self.parse_own_specified(str);
        return self;
    }
};


static_assert(CommonSpecifier::from_str("<").align_kind == Alignment::Kind::Left);
static_assert(CommonSpecifier::from_str(">").align_kind == Alignment::Kind::Right);

struct [[nodiscard]] IntSpecifier final {
    CommonSpecifier super;

    IntRadix::Kind radix;

    static constexpr IntSpecifier from_default() {
        return {
            .super = CommonSpecifier::from_default(),
            .radix = IntRadix::Kind::Default
        };
    }

    [[nodiscard]] constexpr const char * parse_own_specified(std::string_view str){
        auto & self = *this;
        auto begin = str.data();
        auto end = str.data() + str.size();

        // Parse [radix]
        if(begin < end){
            IntRadix::try_from_char(*begin).inspect([&](auto int_radix){
                self.radix = int_radix.kind;
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
        if(begin < end && *begin == '.'){
            begin++;
            uint8_t prec = 0;
            while(begin < end && *begin >= '0' && *begin <= '9'){
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
        if(begin < end && *begin == '.'){
            begin++;
            uint8_t prec = 0;
            while(begin < end && *begin >= '0' && *begin <= '9'){
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

// Tests - CommonSpecifier
static_assert(CommonSpecifier::from_str("<").align_kind == Alignment::Kind::Left);
static_assert(CommonSpecifier::from_str(">").align_kind == Alignment::Kind::Right);
static_assert(CommonSpecifier::from_str("^").align_kind == Alignment::Kind::Center);
static_assert(CommonSpecifier::from_str("=").align_kind == Alignment::Kind::SignAware);
static_assert(CommonSpecifier::from_str("+").sign_kind == Sign::Kind::Always);
static_assert(CommonSpecifier::from_str("-").sign_kind == Sign::Kind::NegativeOnly);
static_assert(CommonSpecifier::from_str(" ").sign_kind == Sign::Kind::Space);
static_assert(CommonSpecifier::from_str("#").explicit_alternate);
static_assert(CommonSpecifier::from_str("010").zero_padding && CommonSpecifier::from_str("010").width == 10);
static_assert(CommonSpecifier::from_str("100").width == 100);
static_assert(CommonSpecifier::from_str(",").thousands_separator);
static_assert(CommonSpecifier::from_str("*<").fill == '*' && CommonSpecifier::from_str("*<").align_kind == Alignment::Kind::Left);
static_assert(CommonSpecifier::from_str("*>").fill == '*' && CommonSpecifier::from_str("*>").align_kind == Alignment::Kind::Right);
static_assert(CommonSpecifier::from_str("+010,").sign_kind == Sign::Kind::Always &&
              CommonSpecifier::from_str("+010,").zero_padding &&
              CommonSpecifier::from_str("+010,").width == 10 &&
              CommonSpecifier::from_str("+010,").thousands_separator);

// Tests - IntSpecifier
static_assert(IntSpecifier::from_str("x").radix == IntRadix::Kind::HexLower);
static_assert(IntSpecifier::from_str("X").radix == IntRadix::Kind::HexUpper);
static_assert(IntSpecifier::from_str("b").radix == IntRadix::Kind::Binary);
static_assert(IntSpecifier::from_str("o").radix == IntRadix::Kind::Octal);
static_assert(IntSpecifier::from_str("d").radix == IntRadix::Kind::Decimal);
static_assert(IntSpecifier::from_str("c").radix == IntRadix::Kind::Char);
static_assert(IntSpecifier::from_str("#x").super.explicit_alternate && IntSpecifier::from_str("#x").radix == IntRadix::Kind::HexLower);
static_assert(IntSpecifier::from_str("#b").super.explicit_alternate && IntSpecifier::from_str("#b").radix == IntRadix::Kind::Binary);
static_assert(IntSpecifier::from_str(">+010x").super.align_kind == Alignment::Kind::Right);
static_assert(IntSpecifier::from_str(">+010x").super.sign_kind == Sign::Kind::Always);
static_assert(!IntSpecifier::from_str(">+010x").super.zero_padding);  // '0' is part of width when align is explicit
static_assert(IntSpecifier::from_str(">+010x").super.width == 10);
static_assert(IntSpecifier::from_str("+010x").super.zero_padding);     // '0' is padding when no explicit align
static_assert(IntSpecifier::from_str("+010x").super.width == 10);
static_assert(IntSpecifier::from_str("010x").super.width == 10 && IntSpecifier::from_str("010x").radix == IntRadix::Kind::HexLower);
static_assert(IntSpecifier::from_str("+,d").super.sign_kind == Sign::Kind::Always && IntSpecifier::from_str("+,d").super.thousands_separator);
static_assert(IntSpecifier::from_str("*>10,x").super.fill == '*' && IntSpecifier::from_str("*>10,x").super.width == 10);

// Tests - FloatingSpecifier
static_assert(FloatingSpecifier::from_str("f").type == FloatingType::Kind::Fixed);
static_assert(FloatingSpecifier::from_str("e").type == FloatingType::Kind::Scientific);
static_assert(FloatingSpecifier::from_str("g").type == FloatingType::Kind::Adaptive);
static_assert(FloatingSpecifier::from_str("a").type == FloatingType::Kind::Hexadecimal);
static_assert(FloatingSpecifier::from_str(".2f").precision == 2 && FloatingSpecifier::from_str(".2f").type == FloatingType::Kind::Fixed);
static_assert(FloatingSpecifier::from_str(".10e").precision == 10 && FloatingSpecifier::from_str(".10e").type == FloatingType::Kind::Scientific);
static_assert(FloatingSpecifier::from_str("#.2f").super.explicit_alternate && FloatingSpecifier::from_str("#.2f").precision == 2);
static_assert(FloatingSpecifier::from_str("+.3g").super.sign_kind == Sign::Kind::Always && FloatingSpecifier::from_str("+.3g").precision == 3);
static_assert(FloatingSpecifier::from_str(">10.5f").super.width == 10 && FloatingSpecifier::from_str(">10.5f").precision == 5);
static_assert(FloatingSpecifier::from_str("010.2f").super.zero_padding && FloatingSpecifier::from_str("010.2f").precision == 2);

// Tests - StringSpecifier
static_assert(StringSpecifier::from_str(".10").precision == 10);
static_assert(StringSpecifier::from_str(".5s").precision == 5);
static_assert(StringSpecifier::from_str(">10.5").super.width == 10 && StringSpecifier::from_str(">10.5").precision == 5);
static_assert(StringSpecifier::from_str("<20.15").super.align_kind == Alignment::Kind::Left && StringSpecifier::from_str("<20.15").super.width == 20);
static_assert(StringSpecifier::from_str("*^30.20").super.fill == '*' && StringSpecifier::from_str("*^30.20").super.width == 30);
static_assert(StringSpecifier::from_str("10").super.width == 10);

// Tests - BoolSpecifier
static_assert(BoolSpecifier::from_str("s").type == BoolType::Kind::Text);
static_assert(BoolSpecifier::from_str("").type == BoolType::Kind::Numeric);
static_assert(BoolSpecifier::from_str(">10s").super.width == 10 && BoolSpecifier::from_str(">10s").type == BoolType::Kind::Text);
static_assert(BoolSpecifier::from_str("<15").super.width == 15 && BoolSpecifier::from_str("<15").type == BoolType::Kind::Numeric);

// Tests - CharSpecifier
static_assert(CharSpecifier::from_str(">10").super.width == 10 && CharSpecifier::from_str(">10").super.align_kind == Alignment::Kind::Right);
static_assert(CharSpecifier::from_str("^5").super.width == 5 && CharSpecifier::from_str("^5").super.align_kind == Alignment::Kind::Center);
static_assert(CharSpecifier::from_str("*<8").super.fill == '*' && CharSpecifier::from_str("*<8").super.width == 8);

// Tests - PointerSpecifier
static_assert(PointerSpecifier::from_str(">20").super.width == 20 && PointerSpecifier::from_str(">20").super.align_kind == Alignment::Kind::Right);
static_assert(PointerSpecifier::from_str("010").super.zero_padding && PointerSpecifier::from_str("010").super.width == 10);
static_assert(PointerSpecifier::from_str("*^16").super.fill == '*' && PointerSpecifier::from_str("*^16").super.width == 16);

}
