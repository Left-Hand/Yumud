#include "strconv2.hpp"

using namespace ymd;
using namespace ymd::strconv2;

namespace {

[[maybe_unused]] static void test_radix(){
    static_assert(IntDeformatter<uint32_t>::parse_radix("0x").unwrap_err() == DestringError::HexBaseOnly);
    static_assert(IntDeformatter<uint32_t>::parse_radix("0b").unwrap_err() == DestringError::BinBaseOnly);
    static_assert(IntDeformatter<uint32_t>::parse_radix("0o").unwrap_err() == DestringError::OctBaseOnly);

    static_assert(IntDeformatter<uint32_t>::parse_radix("0X").unwrap_err() == DestringError::HexBaseOnly);
    static_assert(IntDeformatter<uint32_t>::parse_radix("0B").unwrap_err() == DestringError::BinBaseOnly);
    static_assert(IntDeformatter<uint32_t>::parse_radix("0O").unwrap_err() == DestringError::OctBaseOnly);

    static_assert(IntDeformatter<uint32_t>::parse_radix("0B101").unwrap() == Radix(Radix::Kind::Bin));
    static_assert(IntDeformatter<uint32_t>::parse_radix("0x123").unwrap() == Radix(Radix::Kind::Hex));
    static_assert(IntDeformatter<uint32_t>::parse_radix("0o123").unwrap() == Radix(Radix::Kind::Oct));

    static_assert(IntDeformatter<uint32_t>::parse("0B101").unwrap() == 0b101);
    static_assert(IntDeformatter<uint32_t>::parse("0x123").unwrap() == 0x123);
    static_assert(IntDeformatter<uint32_t>::parse("0o123").unwrap() == 0123);
}
[[maybe_unused]] static void test_hex(){


    static_assert(IntDeformatter<uint32_t>::parse_hex_no_show_base("ffff").unwrap() 
        == 0xffff);
    static_assert(IntDeformatter<uint32_t>::parse_hex_no_show_base("ffffffff").unwrap() 
        == 0xffffffff);
    static_assert(IntDeformatter<uint64_t>::parse_hex_no_show_base("ffffffffffffffff").unwrap() 
        == 0xffffffff'ffffffff);


    static_assert(IntDeformatter<uint8_t>::parse_hex_no_show_base("00").unwrap() == 0x00);
    static_assert(IntDeformatter<uint8_t>::parse_hex_no_show_base("f0").unwrap() == 0xf0);
    static_assert(IntDeformatter<uint8_t>::parse_hex_no_show_base("ff").unwrap() == 0xff);
    static_assert(IntDeformatter<uint8_t>::parse_hex_no_show_base("fff").unwrap_err() == DestringError::StrTooLong);
}

[[maybe_unused]] static void test_oct(){
    // 八进制基础测试
    static_assert(IntDeformatter<uint32_t>::parse_oct_no_show_base("37777777777").unwrap() 
        == 0xffffffff);  // 八进制 37777777777 = 十六进制 ffffffff

    static_assert(IntDeformatter<uint64_t>::parse_oct_no_show_base("1777777777777777777777").unwrap() 
        == 0xffffffff'ffffffff);  // 64位全1

    // 8位类型测试
    static_assert(IntDeformatter<uint8_t>::parse_oct_no_show_base("000").unwrap() == 0x00);
    static_assert(IntDeformatter<uint8_t>::parse_oct_no_show_base("377").unwrap() == 0xff);  // 377 octal = 255 decimal
    static_assert(IntDeformatter<uint8_t>::parse_oct_no_show_base("400").unwrap_err() == DestringError::Overflow);  // 400 > 377

    // 边界和错误测试
    static_assert(IntDeformatter<uint8_t>::parse_oct_no_show_base("").unwrap_err() == DestringError::EmptyString);
    static_assert(IntDeformatter<uint8_t>::parse_oct_no_show_base("1234").unwrap_err() == DestringError::StrTooLong);  // 4 > max_digits(3)
    static_assert(IntDeformatter<uint8_t>::parse_oct_no_show_base("8").unwrap_err() == DestringError::DigitExceedsOct);  // 无效八进制字符
    static_assert(IntDeformatter<uint8_t>::parse_oct_no_show_base("9").unwrap_err() == DestringError::DigitExceedsOct);

    // 前导零测试
    static_assert(IntDeformatter<uint16_t>::parse_oct_no_show_base("0017777").unwrap_err() == DestringError::StrTooLong);  // 前导零
    static_assert(IntDeformatter<uint16_t>::parse_oct_no_show_base("17777").unwrap() == 017777);    // 无前导零

    // 各种值测试
    static_assert(IntDeformatter<uint16_t>::parse_oct_no_show_base("7777").unwrap() == 07777);
    static_assert(IntDeformatter<uint32_t>::parse_oct_no_show_base("1234567").unwrap() == 01234567);
    static_assert(IntDeformatter<uint32_t>::parse_oct_no_show_base("7654321").unwrap() == 07654321);

    // 最大长度测试
    static_assert(IntDeformatter<uint32_t>::parse_oct_no_show_base("77777777777").unwrap_err() 
        == DestringError::Overflow);  // 超过32位
}

[[maybe_unused]] static void test_bin(){
    // 二进制基础测试
    static_assert(IntDeformatter<uint32_t>::parse_bin_no_show_base("11111111111111111111111111111111").unwrap() 
        == 0xffffffff);  // 32个1

    static_assert(IntDeformatter<uint64_t>::parse_bin_no_show_base("1111111111111111111111111111111111111111111111111111111111111111").unwrap() 
        == 0xffffffff'ffffffff);  // 64个1

    // 8位类型测试
    static_assert(IntDeformatter<uint8_t>::parse_bin_no_show_base("00000000").unwrap() == 0x00);
    static_assert(IntDeformatter<uint8_t>::parse_bin_no_show_base("11110000").unwrap() == 0xf0);
    static_assert(IntDeformatter<uint8_t>::parse_bin_no_show_base("11111111").unwrap() == 0xff);
    static_assert(IntDeformatter<uint8_t>::parse_bin_no_show_base("111111111").unwrap_err() == DestringError::StrTooLong);  // 9 > 8

    // 边界和错误测试
    static_assert(IntDeformatter<uint8_t>::parse_bin_no_show_base("").unwrap_err() == DestringError::EmptyString);
    static_assert(IntDeformatter<uint8_t>::parse_bin_no_show_base("2").unwrap_err() == DestringError::DigitExceedsBin);  // 无效二进制字符
    static_assert(IntDeformatter<uint8_t>::parse_bin_no_show_base("a").unwrap_err() == DestringError::InvalidChar);
    static_assert(IntDeformatter<uint8_t>::parse_bin_no_show_base("10101012").unwrap_err() == DestringError::DigitExceedsBin);

    // 各种模式测试
    static_assert(IntDeformatter<uint8_t>::parse_bin_no_show_base("10101010").unwrap() == 0xaa);
    static_assert(IntDeformatter<uint8_t>::parse_bin_no_show_base("01010101").unwrap() == 0x55);
    static_assert(IntDeformatter<uint16_t>::parse_bin_no_show_base("1010101010101010").unwrap() == 0xaaaa);
    static_assert(IntDeformatter<uint16_t>::parse_bin_no_show_base("0101010101010101").unwrap() == 0x5555);

    // 前导零测试
    static_assert(IntDeformatter<uint16_t>::parse_bin_no_show_base("0011111111111111").unwrap() == 0x3fff);
    static_assert(IntDeformatter<uint16_t>::parse_bin_no_show_base("1111111111111111").unwrap() == 0xffff);

    // 混合测试
    static_assert(IntDeformatter<uint32_t>::parse_bin_no_show_base("11001100110011001100110011001100").unwrap() == 0xcccccccc);
    static_assert(IntDeformatter<uint32_t>::parse_bin_no_show_base("00110011001100110011001100110011").unwrap() == 0x33333333);

    // 最大长度边缘测试
    static_assert(IntDeformatter<uint8_t>::parse_bin_no_show_base("11111111").is_ok());
    static_assert(IntDeformatter<uint16_t>::parse_bin_no_show_base("1111111111111111").is_ok());
    static_assert(IntDeformatter<uint32_t>::parse_bin_no_show_base("11111111111111111111111111111111").is_ok());
}
}