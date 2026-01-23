// unit_test.cpp
#include "../bits_set.hpp"  // 假设你的头文件名为 bitset.hpp
#include <type_traits>

using namespace ymd;
// 测试用例分组

namespace {

// 基础类型和约束测试
[[maybe_unused]] void basic_tests() {
    // 测试类型别名
    static_assert(std::is_same_v<BitsSet<8>::D, uint8_t>);
    static_assert(std::is_same_v<BitsSet<16>::D, uint16_t>);
    static_assert(std::is_same_v<BitsSet<32>::D, uint32_t>);

    // 测试掩码计算
    static_assert(BitsSet<8>::BITS_MASK == 0xFF);
    static_assert(BitsSet<16>::BITS_MASK == 0xFFFF);
    static_assert(BitsSet<5>::BITS_MASK == 0b11111);
    static_assert(BitsSet<1>::BITS_MASK == 0b1);
} // namespace BasicTests

// 构造和验证测试
[[maybe_unused]] void construction_tests() {
    // 测试安全构造
    constexpr auto bs1 = BitsSet<8>::from_bits(0x12);
    static_assert(bs1.to_bits() == 0x12);

    constexpr auto bs2 = BitsSet<16>::from_bits(0x1234);
    static_assert(bs2.to_bits() == 0x1234);

    // 测试边界构造
    constexpr auto bs3 = BitsSet<8>::from_bits_bounded<uint16_t>(0x123);
    static_assert(bs3.to_bits() == 0x23);  // 高位被截断

    constexpr auto bs4 = BitsSet<5>::from_bits_bounded(0b111111);
    static_assert(bs4.to_bits() == 0b11111);

    // 测试无检查构造
    constexpr auto bs5 = BitsSet<8>::from_bits_unchecked(0xAA);
    static_assert(bs5.to_bits() == 0xAA);

    // 测试尝试构造
    constexpr auto opt1 = BitsSet<8>::try_from_bits(0x12);
    static_assert(opt1.is_some());
    static_assert((opt1).unwrap().to_bits() == 0x12);

    constexpr auto opt2 = BitsSet<8>::try_from_bits(0x123);
    static_assert(opt2.is_none());  // 超出范围应该失败
} // namespace ConstructionTests

// 位操作测试
[[maybe_unused]] void bit_operation_tests() {
    // 测试位访问
    constexpr auto bs1 = BitsSet<8>::from_bits(0b10101010);
    static_assert(bs1[0] == false);
    static_assert(bs1[1] == true);
    static_assert(bs1[2] == false);
    static_assert(bs1[3] == true);
    static_assert(bs1[7] == true);

    // 测试 test 方法
    static_assert(bs1.test(0) == false);
    static_assert(bs1.test(1) == true);
    static_assert(bs1.test(7) == true);

    // 测试 count_ones
    static_assert(bs1.count_ones() == 4);

    constexpr auto bs2 = BitsSet<8>::from_bits(0b11110000);
    static_assert(bs2.count_ones() == 4);

    constexpr auto bs3 = BitsSet<8>::from_bits(0b00000000);
    static_assert(bs3.count_ones() == 0);

    constexpr auto bs4 = BitsSet<8>::from_bits(0b11111111);
    static_assert(bs4.count_ones() == 8);

    // 测试 width
    static_assert(BitsSet<1>::width() == 1);
    static_assert(BitsSet<8>::width() == 8);
    static_assert(BitsSet<16>::width() == 16);
    static_assert(BitsSet<32>::width() == 32);
} // namespace BitOperationTests

// 连接操作测试
[[maybe_unused]] void connection_tests() {
    constexpr auto bs1 = BitsSet<4>::from_bits(0b1010);
    constexpr auto bs2 = BitsSet<4>::from_bits(0b1100);
    
    constexpr auto connected = bs1.connect(bs2);
    static_assert(connected.to_bits() == 0b10101100);
    static_assert(connected.width() == 8);

    // 测试不同大小的连接
    constexpr auto bs3 = BitsSet<8>::from_bits(0xAB);
    constexpr auto bs4 = BitsSet<4>::from_bits(0xC);
    constexpr auto connected2 = bs3.connect(bs4);
    static_assert(connected2.to_bits() == 0xABC);
    static_assert(connected2.width() == 12);

    constexpr auto bs5 = BitsSet<2>::from_bits(0b10);
    constexpr auto bs6 = BitsSet<3>::from_bits(0b101);
    constexpr auto connected3 = bs5.connect(bs6);
    static_assert(connected3.to_bits() == 0b10101);
    static_assert(connected3.width() == 5);
} // namespace ConnectionTests

// 分割操作测试
[[maybe_unused]] void split_tests() {
    constexpr auto bs1 = BitsSet<8>::from_bits(0b10101100);
    // 测试等分分割
    static constexpr auto split_result1 = bs1.split<4>();
    static_assert(std::get<0>(split_result1).to_bits() == 0b1010);
    static_assert(std::get<1>(split_result1).to_bits() == 0b1100);
    static_assert(std::get<0>(split_result1).width() == 4);
    static_assert(std::get<1>(split_result1).width() == 4);

    // 测试不等分分割
    static constexpr auto split_result2 = bs1.split<2>();
    static_assert(std::get<0>(split_result2).to_bits() == 0b10);
    static_assert(std::get<1>(split_result2).to_bits() == 0b101100);
    static_assert(std::get<0>(split_result2).width() == 2);
    static_assert(std::get<1>(split_result2).width() == 6);

    static constexpr auto split_result3 = bs1.split<6>();
    static_assert(std::get<0>(split_result3).to_bits() == 0b101011);
    static_assert(std::get<1>(split_result3).to_bits() == 0b00);
    static_assert(std::get<0>(split_result3).width() == 6);
    static_assert(std::get<1>(split_result3).width() == 2);

} // namespace SplitTests

// 操作符测试
[[maybe_unused]] void operator_tests() {
    constexpr auto bs1 = BitsSet<8>::from_bits(0b10101010);
    constexpr auto bs2 = BitsSet<8>::from_bits(0b11001100);

    // 测试相等操作符
    static_assert(bs1 == bs1);
    static_assert(bs1 != bs2);

    // 测试位或操作符
    constexpr auto or_result = bs1 | bs2;
    static_assert(or_result.to_bits() == 0b11101110);

    // 测试位与操作符
    constexpr auto and_result = bs1 & bs2;
    static_assert(and_result.to_bits() == 0b10001000);

    // 测试位异或操作符
    constexpr auto xor_result = bs1 ^ bs2;
    static_assert(xor_result.to_bits() == 0b01100110);

    // 测试位非操作符
    constexpr auto not_result = ~bs1;
    static_assert(not_result.to_bits() == 0b01010101);  // 在8位掩码下

    // 测试组合操作
    constexpr auto combined = (bs1 | bs2) & BitsSet<8>::from_bits(0b11110000);
    static_assert(combined.to_bits() == 0b11100000);
} // namespace OperatorTests

// 边界情况测试
[[maybe_unused]] void edge_case_tests() {
    // 测试最小大小
    constexpr auto bs1 = BitsSet<1>::from_bits(0b1);
    static_assert(bs1[0] == true);
    static_assert(bs1.count_ones() == 1);
    static_assert(bs1.width() == 1);

    constexpr auto bs0 = BitsSet<1>::from_bits(0b0);
    static_assert(bs0[0] == false);
    static_assert(bs0.count_ones() == 0);

    // 测试最大允许大小
    constexpr auto bs32 = BitsSet<32>::from_bits(0xFFFFFFFF);
    static_assert(bs32.count_ones() == 32);
    static_assert(bs32.width() == 32);

    // 测试所有位设置
    constexpr auto bs8_all = BitsSet<8>::from_bits(0xFF);
    static_assert(bs8_all.count_ones() == 8);
    static_assert(bs8_all.bitwise_not().to_bits() == 0);  // 取反后在掩码下为0

    // 测试无位设置
    constexpr auto bs8_none = BitsSet<8>::from_bits(0x00);
    static_assert(bs8_none.count_ones() == 0);
    static_assert(bs8_none.bitwise_not().to_bits() == 0xFF);
} // namespace EdgeCaseTests

// 编译时计算测试
[[maybe_unused]] void compile_time_tests() {

#if 0
// 所有操作都应该是 constexpr
constexpr auto test_compile_time = [] -> size_t{
    auto bs1 = BitsSet<8>::from_bits(0xAA);
    
    auto connected = bs1.connect(BitsSet<4>::from_bits(0xF));
    auto [high, low] = connected.split<6>();
    
    auto result = BitsSet<10>(high | low) & BitsSet<10>::from_bits(0x3FF);
    return result.count_ones();
};

static_assert(test_compile_time() == 5);


// 测试代码
constexpr auto bs32 = BitsSet<32>::from_bits(0x12345678);

// 原来的二分分割（仍然可用）
constexpr auto [high16, low16] = bs32.split<16>();
static_assert(high16.to_bits() == 0x1234);
static_assert(low16.to_bits() == 0x5678);

// 新的多参数分割：分成4个8位片段
constexpr auto [part1, part2, part3, part4] = bs32.split<8, 8, 8>();
static_assert(part1.to_bits() == 0x12);
static_assert(part2.to_bits() == 0x34);
static_assert(part3.to_bits() == 0x56);
static_assert(part4.to_bits() == 0x78);

// 分成不等大小的片段
constexpr auto [a, b, c, d] = bs32.split<4, 8, 12>();
static_assert(a.to_bits() == 0x1);    // 4 bits: 0x1
static_assert(b.to_bits() == 0x23);   // 8 bits: 0x23  
static_assert(c.to_bits() == 0x456);  // 12 bits: 0x456
static_assert(d.to_bits() == 0x78);   // 剩余8 bits: 0x78

// 编译时验证
constexpr auto bs24 = BitsSet<24>::from_bits(0xABCDEF);
constexpr auto [x, y, z] = bs24.split<6, 10>();
static_assert(x.to_bits() == 0x2A);   // 6 bits: 0xABCDEF >> 18 = 0x2A
static_assert(y.to_bits() == 0x2F7);  // 10 bits: (0xABCDEF >> 8) & 0x3FF = 0x2F7
static_assert(z.to_bits() == 0xEF);   // 剩余8 bits: 0xEF
#endif
} // namespace CompileTimeTests



} // namespace BitsetTests