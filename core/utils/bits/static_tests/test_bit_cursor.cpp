#include "../bit_cursor.hpp"
#include "core/utils/Result.hpp"

using namespace ymd;


namespace{



[[maybe_unused]] static void test_bit_cursor(){
    // 测试1: 基本位写入 - 4位 + 4位
    {
        [[maybe_unused]] constexpr auto arr = [] {
            std::array<uint8_t, 3> raw = {0};
            auto cursor = BitCursor{raw.data(), 0};
            constexpr uint8_t PATTERN = 0x0a;
            cursor.push_bits(&PATTERN, 4);
            cursor.push_bits(&PATTERN, 4);
            cursor.push_bits(&PATTERN, 4);
            return raw;
        }();
        static_assert(arr[0] == 0xaa);
        static_assert(arr[1] == 0x0a);
    }

    // 测试2: 字节对齐的整数写入
    {
        [[maybe_unused]] constexpr auto arr = [] {
            std::array<uint8_t, 4> raw = {0};
            auto cursor = BitCursor{raw.data(), 0};
            cursor.push_int<uint32_t>(0x12345678);
            return raw;
        }();
        // 小端序假设: 0x78 0x56 0x34 0x12
        static_assert(arr[0] == 0x78);
        static_assert(arr[1] == 0x56);
        static_assert(arr[2] == 0x34);
        static_assert(arr[3] == 0x12);
    }

    // 测试3: 非字节对齐的整数写入 (起始于位3)
    {
        [[maybe_unused]] constexpr auto arr = [] {
            std::array<uint8_t, 5> raw = {0};
            auto cursor = BitCursor{raw.data(), 3};  // 从第3位开始
            cursor.push_int<uint16_t>(0xabcd);
            return raw;
        }();
        // 手动验证跨字节边界写入
        // 这里需要根据bit_cursor_store_bits的实现来验证
    }

    // 测试4: bitset写入 - 标准大小
    {
        [[maybe_unused]] constexpr auto arr = [] {
            std::array<uint8_t, 2> raw = {0};
            auto cursor = BitCursor{raw.data(), 0};
            std::bitset<12> bs;
            for (size_t i = 0; i < 12; i += 2) {
                bs.set(i);  // 设置偶数位
            }
            cursor.push_bitset(bs);
            return raw;
        }();
        // 位0,2,4,6,8,10被设置
        static_assert(arr[0] == 0b01010101);  // 位0-7
        static_assert(arr[1] == 0b00000101);  // 位8-11
    }

    // 测试5: bitset写入 - 非整字节大小 (7位)
    {
        [[maybe_unused]] constexpr auto arr = [] {
            std::array<uint8_t, 1> raw = {0};
            auto cursor = BitCursor{raw.data(), 0};
            std::bitset<7> bs;
            for (size_t i = 0; i < 7; ++i) {
                bs.set(i);
            }
            cursor.push_bitset(bs);
            return raw;
        }();
        static_assert(arr[0] == 0b01111111);  // 只有低7位被设置
    }

    // 测试6: 跨越字节边界的bitset
    {
        [[maybe_unused]] constexpr auto arr = [] {
            std::array<uint8_t, 3> raw = {0};
            auto cursor = BitCursor{raw.data(), 4};  // 从字节0的第4位开始
            std::bitset<12> bs;
            bs.set();  // 全部设置
            cursor.push_bitset(bs);
            return raw;
        }();
        // 需要根据实现验证
    }

    // 测试7: 对齐检测 - 字节对齐
    {
        constexpr bool test = [] {
            std::array<uint8_t, 4> raw = {0};
            BitCursor cursor{raw.data(), 0};
            if (!cursor.is_aligned_to_byte()) return false;
            cursor.push_bits(raw.data(), 8);  // 写入8位
            return cursor.is_aligned_to_byte();
        }();
        static_assert(test);
    }

    // 测试8: 对齐检测 - 非字节对齐
    {
        constexpr bool test = [] {
            std::array<uint8_t, 4> raw = {0};
            BitCursor cursor{raw.data(), 3};
            return !cursor.is_aligned_to_byte();  // 应该返回false
        }();
        static_assert(test);
    }

    // 测试9: 连续写入 - 混合类型
    {
        [[maybe_unused]] constexpr auto arr = [] {
            std::array<uint8_t, 8> raw = {0};
            auto cursor = BitCursor{raw.data(), 0};
            cursor.push_int<uint8_t>(0x0f, 4);      // 写入4位: 0b1111
            cursor.push_int<uint8_t>(0x12);    // 写入8位: 0x12
            cursor.push_int<uint8_t>(0x01, 1);       // 写入1位: 1
            cursor.push_bitset<3>(std::bitset<3>("101")); // 写入3位: 101
            return raw;
        }();
        // 验证按位布局 (取决于具体实现)
    }

    // 测试10: 大端序模拟 (通过手动构造)
    {
        [[maybe_unused]] constexpr auto arr = [] {
            std::array<uint8_t, 4> raw = {0};
            auto cursor = BitCursor{raw.data(), 0};
            // 手动以大端序方式写入
            constexpr uint8_t big_endian[] = {0x12, 0x34, 0x56, 0x78};
            for (size_t i = 0; i < 4; ++i) {
                cursor.push_bits(&big_endian[i], 8);
            }
            return raw;
        }();
        static_assert(arr[0] == 0x12);
        static_assert(arr[1] == 0x34);
        static_assert(arr[2] == 0x56);
        static_assert(arr[3] == 0x78);
    }

    // 测试11: 边界压力测试 - 写入大量单比特
    {
        [[maybe_unused]] constexpr auto arr = [] {
            std::array<uint8_t, 16> raw = {0};
            auto cursor = BitCursor{raw.data(), 0};
            for (int i = 0; i < 128; ++i) {
                uint8_t bit = (i % 2 == 0) ? 1 : 0;
                cursor.push_bits(&bit, 1);
            }
            return raw;
        }();
        static_assert(arr[0] == 0x55);
        static_assert(arr[1] == 0x55);
        static_assert(arr[2] == 0x55);
        static_assert(arr[3] == 0x55);
    }

    // 测试11: 边界压力测试 - 写入大量单比特（交替 1/0）
    {
        [[maybe_unused]] constexpr auto arr = [] {
            std::array<uint8_t, 16> raw = {0};
            auto cursor = BitCursor{raw.data(), 0};
            for (int i = 0; i < 128; ++i) {
                uint8_t bit = (i % 2 == 0) ? 1 : 0; // 1,0,1,0,...
                cursor.push_bits(&bit, 1);
            }
            return raw;
        }();
        static_assert(arr[0] == 0x55);
        static_assert(arr[1] == 0x55);
        static_assert(arr[2] == 0x55);
        static_assert(arr[3] == 0x55);
        static_assert(arr[15] == 0x55);
    }

    // 🔹 新增测试12: 空写入（0位）不应改变状态
    {
        constexpr bool test = [] {
            std::array<uint8_t, 1> raw = {0xFF};
            BitCursor cursor{raw.data(), 3};
            cursor.push_bits(raw.data(), 0); // 写入0位
            return raw[0] == 0xFF && cursor.bit_offset == 3;
        }();
        static_assert(test);
    }

    // 🔹 新增测试13: 对齐后继续写入整数
    {
        [[maybe_unused]] constexpr auto arr = [] {
            std::array<uint8_t, 4> raw = {0};
            BitCursor cursor{raw.data(), 0}; // 起始于 bit4
            cursor.push_int<uint8_t>(0x0f, 4);     // 填满第一个字节 → now aligned
            cursor.push_int<uint16_t>(0x1234); // 应从小端写入到 byte1 和 byte2
            return raw;
        }();
        static_assert(arr[0] == 0x4f);
        static_assert(arr[1] == 0x23);
        static_assert(arr[2] == 0x01);
        static_assert(arr[3] == 0x00);
    }

    // 🔹 新增测试14: 跨3字节的 bitset (20位全1)
    {
        [[maybe_unused]] constexpr auto arr = [] {
            std::array<uint8_t, 3> raw = {0};
            BitCursor cursor{raw.data(), 0};
            std::bitset<20> bs;
            bs.set(); // 全1
            cursor.push_bitset(bs);
            return raw;
        }();
        // 20位全1：byte0=0xFF, byte1=0xFF, byte2=0x0F (低4位为1)
        static_assert(arr[0] == 0xFF);
        static_assert(arr[1] == 0xFF);
        static_assert(arr[2] == 0x0F);
    }
}


}