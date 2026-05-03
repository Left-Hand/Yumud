#include "strconv.hpp"

using namespace ymd;
using namespace ymd::strconv;

template<typename D>
static constexpr D err_bits(const D a, const D b){
    if(b > a) return static_cast<D>(b - a);
    else return static_cast<D>(a - b);
}


namespace {

[[maybe_unused]] static void test_radix(){
    static_assert(IntDeformatterGeneric::parse_radix("0x").unwrap_err() == DeformatError::HexBaseOnly);
    static_assert(IntDeformatterGeneric::parse_radix("0b").unwrap_err() == DeformatError::BinBaseOnly);
    static_assert(IntDeformatterGeneric::parse_radix("0o").unwrap_err() == DeformatError::OctBaseOnly);

    static_assert(IntDeformatterGeneric::parse_radix("0X").unwrap_err() == DeformatError::HexBaseOnly);
    static_assert(IntDeformatterGeneric::parse_radix("0B").unwrap_err() == DeformatError::BinBaseOnly);
    static_assert(IntDeformatterGeneric::parse_radix("0O").unwrap_err() == DeformatError::OctBaseOnly);

    static_assert(IntDeformatterGeneric::parse_radix("0B101").unwrap() == Radix(Radix::Kind::Bin));
    static_assert(IntDeformatterGeneric::parse_radix("0x123").unwrap() == Radix(Radix::Kind::Hex));
    static_assert(IntDeformatterGeneric::parse_radix("0o123").unwrap() == Radix(Radix::Kind::Oct));

    static_assert(IntDeformatter<uint32_t>::parse("0B101").unwrap() == 0b101);
    static_assert(IntDeformatter<uint32_t>::parse("0x123").unwrap() == 0x123);
    static_assert(IntDeformatter<uint32_t>::parse("0o123").unwrap() == 0123);
}
[[maybe_unused]] static void test_hex(){


    static_assert(IntDeformatterGeneric::parse_bare_hex32("ffff", str::IntTypeErased::from<uint32_t>()).unwrap() 
        == 0xffff);
    static_assert(IntDeformatterGeneric::parse_bare_hex32("ffffffff", str::IntTypeErased::from<uint32_t>()).unwrap() 
        == 0xffffffff);


    static_assert(IntDeformatterGeneric::parse_bare_hex32("00", str::IntTypeErased::from<uint8_t>()).unwrap() == 0x00);
    static_assert(IntDeformatterGeneric::parse_bare_hex32("f0", str::IntTypeErased::from<uint8_t>()).unwrap() == 0xf0);
    static_assert(IntDeformatterGeneric::parse_bare_hex32("ff", str::IntTypeErased::from<uint8_t>()).unwrap() == 0xff);
    static_assert(IntDeformatterGeneric::parse_bare_hex32("fff", str::IntTypeErased::from<uint8_t>()).unwrap_err() == DeformatError::StrTooLong);
}

[[maybe_unused]] static void test_oct(){
    // 八进制基础测试
    static_assert(IntDeformatterGeneric::parse_bare_oct32("37777777777", str::IntTypeErased::from<uint32_t>()).unwrap() 
        == 0xffffffff);  // 八进制 37777777777 = 十六进制 ffffffff

    // static_assert(IntDeformatterGeneric::parse_bare_oct32("1777777777777777777777", str::IntTypeErased::from<uint64_t>()).unwrap() 
    //     == 0xffffffff'ffffffff);  // 64位全1

    // 8位类型测试
    static_assert(IntDeformatterGeneric::parse_bare_oct32("000", str::IntTypeErased::from<uint8_t>()).unwrap() == 0x00);
    static_assert(IntDeformatterGeneric::parse_bare_oct32("377", str::IntTypeErased::from<uint8_t>()).unwrap() == 0xff);  // 377 octal = 255 decimal
    static_assert(IntDeformatterGeneric::parse_bare_oct32("400", str::IntTypeErased::from<uint8_t>()).unwrap_err() == DeformatError::Overflow);  // 400 > 377

    // 边界和错误测试
    static_assert(IntDeformatterGeneric::parse_bare_oct32("", str::IntTypeErased::from<uint8_t>()).unwrap_err() == DeformatError::EmptyString);
    static_assert(IntDeformatterGeneric::parse_bare_oct32("1234", str::IntTypeErased::from<uint8_t>()).unwrap_err() == DeformatError::StrTooLong);  // 4 > max_digits(3)
    static_assert(IntDeformatterGeneric::parse_bare_oct32("8", str::IntTypeErased::from<uint8_t>()).unwrap_err() == DeformatError::DigitExceedsOct);  // 无效八进制字符
    static_assert(IntDeformatterGeneric::parse_bare_oct32("9", str::IntTypeErased::from<uint8_t>()).unwrap_err() == DeformatError::DigitExceedsOct);

    // 前导零测试
    static_assert(IntDeformatterGeneric::parse_bare_oct32("0017777", str::IntTypeErased::from<uint16_t>()).unwrap_err() == DeformatError::StrTooLong);  // 前导零
    static_assert(IntDeformatterGeneric::parse_bare_oct32("17777", str::IntTypeErased::from<uint16_t>()).unwrap() == 017777);    // 无前导零

    // 各种值测试
    static_assert(IntDeformatterGeneric::parse_bare_oct32("7777", str::IntTypeErased::from<uint16_t>()).unwrap() == 07777);
    static_assert(IntDeformatterGeneric::parse_bare_oct32("1234567", str::IntTypeErased::from<uint32_t>()).unwrap() == 01234567);
    static_assert(IntDeformatterGeneric::parse_bare_oct32("7654321", str::IntTypeErased::from<uint32_t>()).unwrap() == 07654321);

    // 最大长度测试
    static_assert(IntDeformatterGeneric::parse_bare_oct32("77777777777", str::IntTypeErased::from<uint32_t>()).unwrap_err() 
        == DeformatError::Overflow);  // 超过32位
}

[[maybe_unused]] static void test_bin(){
    // 二进制基础测试
    static_assert(IntDeformatterGeneric::parse_bare_bin32("11111111111111111111111111111111", str::IntTypeErased::from<uint32_t>()).unwrap() 
        == 0xffffffff);  // 32个1

    // static_assert(IntDeformatterGeneric::parse_bare_bin32("1111111111111111111111111111111111111111111111111111111111111111", str::IntTypeErased::from<uint64_t>()).unwrap() 
    //     == 0xffffffff'ffffffff);  // 64个1

    // 8位类型测试
    static_assert(IntDeformatterGeneric::parse_bare_bin32("00000000", str::IntTypeErased::from<uint8_t>()).unwrap() == 0x00);
    static_assert(IntDeformatterGeneric::parse_bare_bin32("11110000", str::IntTypeErased::from<uint8_t>()).unwrap() == 0xf0);
    static_assert(IntDeformatterGeneric::parse_bare_bin32("11111111", str::IntTypeErased::from<uint8_t>()).unwrap() == 0xff);
    static_assert(IntDeformatterGeneric::parse_bare_bin32("111111111", str::IntTypeErased::from<uint8_t>()).unwrap_err() == DeformatError::StrTooLong);  // 9 > 8

    // 边界和错误测试
    static_assert(IntDeformatterGeneric::parse_bare_bin32("", str::IntTypeErased::from<uint8_t>()).unwrap_err() == DeformatError::EmptyString);
    static_assert(IntDeformatterGeneric::parse_bare_bin32("2", str::IntTypeErased::from<uint8_t>()).unwrap_err() == DeformatError::DigitExceedsBin);  // 无效二进制字符
    static_assert(IntDeformatterGeneric::parse_bare_bin32("a", str::IntTypeErased::from<uint8_t>()).unwrap_err() == DeformatError::InvalidChar);
    static_assert(IntDeformatterGeneric::parse_bare_bin32("10101012", str::IntTypeErased::from<uint8_t>()).unwrap_err() == DeformatError::DigitExceedsBin);

    // 各种模式测试
    static_assert(IntDeformatterGeneric::parse_bare_bin32("10101010", str::IntTypeErased::from<uint8_t>()).unwrap() == 0xaa);
    static_assert(IntDeformatterGeneric::parse_bare_bin32("01010101", str::IntTypeErased::from<uint8_t>()).unwrap() == 0x55);
    static_assert(IntDeformatterGeneric::parse_bare_bin32("1010101010101010", str::IntTypeErased::from<uint16_t>()).unwrap() == 0xaaaa);
    static_assert(IntDeformatterGeneric::parse_bare_bin32("0101010101010101", str::IntTypeErased::from<uint16_t>()).unwrap() == 0x5555);

    // 前导零测试
    static_assert(IntDeformatterGeneric::parse_bare_bin32("0011111111111111", str::IntTypeErased::from<uint16_t>()).unwrap() == 0x3fff);
    static_assert(IntDeformatterGeneric::parse_bare_bin32("1111111111111111", str::IntTypeErased::from<uint16_t>()).unwrap() == 0xffff);

    // 混合测试
    static_assert(IntDeformatterGeneric::parse_bare_bin32("11001100110011001100110011001100", str::IntTypeErased::from<uint32_t>()).unwrap() == 0xcccccccc);
    static_assert(IntDeformatterGeneric::parse_bare_bin32("00110011001100110011001100110011", str::IntTypeErased::from<uint32_t>()).unwrap() == 0x33333333);

    // 最大长度边缘测试
    static_assert(IntDeformatterGeneric::parse_bare_bin32("11111111", str::IntTypeErased::from<uint8_t>()).is_ok());
    static_assert(IntDeformatterGeneric::parse_bare_bin32("1111111111111111", str::IntTypeErased::from<uint16_t>()).is_ok());
    static_assert(IntDeformatterGeneric::parse_bare_bin32("11111111111111111111111111111111", str::IntTypeErased::from<uint32_t>()).is_ok());
}


[[maybe_unused]] static void test_fixed(){
    static_assert(err_bits(FixedPointDeformatter<16, int32_t>::parse("1.70000")
        .unwrap().to_bits(), (1.7_iq16).to_bits()) < 2);
    static_assert(err_bits(FixedPointDeformatter<16, int32_t>::parse("0.90000")
        .unwrap().to_bits(), (0.9_iq16).to_bits()) < 2);
    static_assert(err_bits(FixedPointDeformatter<16, int32_t>::parse("-0.30000")
        .unwrap().to_bits(), (-0.3_iq16).to_bits()) < 2);
    static_assert(err_bits(FixedPointDeformatter<16, int32_t>::parse("-0.0005")
        .unwrap().to_bits(), (-0.0005_iq16).to_bits()) < 2);
    static_assert(err_bits(FixedPointDeformatter<16, int32_t>::parse("-1.7")
        .unwrap().to_bits(), (-1.7_iq16).to_bits()) < 2);
    static_assert(err_bits(FixedPointDeformatter<16, int32_t>::parse("11111.7")
        .unwrap().to_bits(), (11111.7_iq16).to_bits()) < 2);
    static_assert(FixedPointDeformatter<16, int32_t>::parse("41111.5").unwrap_err() == DeformatError::Overflow);
    static_assert(FixedPointDeformatter<16, int32_t>::parse("-41111.5").unwrap_err() == DeformatError::Underflow);
    static_assert(FixedPointDeformatter<16, uint32_t>::parse("-41111.5").unwrap_err() == DeformatError::NegForUnsigned);
    static_assert(FixedPointDeformatter<16, int32_t>::DIGIT_MAX == 32767);

}
}