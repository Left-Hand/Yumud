
#include "../fmt_specifiers.hpp"

using namespace ymd;
using namespace ymd::str;
using namespace ymd::fmt_specifiers;

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