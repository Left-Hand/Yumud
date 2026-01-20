#include "uchars_view.hpp"

using namespace ymd;
using namespace ymd::str;

namespace {

[[maybe_unused]]static void test_uchars_view(){

    {
        static constexpr std::array<uint8_t, 8> bytes = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        constexpr auto uchars = std::span(bytes);
        constexpr auto ustr = UCharsView<10>::from_uchars(uchars);
        static_assert(ustr.length() == 8);
        static_assert(ustr[0] == 0x01);
        static_assert(ustr[7] == 0x08);
    }

    {
        static constexpr std::array<uint8_t, 8> bytes = {0x01, 0x02, 0x0, 0x04, 0, 0, 0};
        constexpr auto uchars = std::span(bytes);
        constexpr auto ustr = UCharsView<10>::from_uchars(uchars);
        static_assert(ustr.length() == 2);
        static_assert(ustr[0] == 0x01);
        static_assert(ustr[1] == 0x02);
    }

    {
        static constexpr std::array<uint8_t, 1> bytes = {0x01};
        constexpr auto uchars = std::span(bytes);
        constexpr auto ustr = UCharsView<10>::from_uchars(uchars);
        static_assert(ustr.length() == 1);
        static_assert(ustr[0] == 0x01);
    }

    {
        static constexpr std::array<uint8_t, 1> bytes = {0x00};
        constexpr auto uchars = std::span(bytes);
        constexpr auto ustr = UCharsView<10>::from_uchars(uchars);
        static_assert(ustr.length() == 0);
    }

    {
        static constexpr std::array<uint8_t, 0> bytes = {};
        constexpr auto uchars = std::span(bytes);
        constexpr auto ustr = UCharsView<10>::from_uchars(uchars);
        static_assert(ustr.length() == 0);
    }

    // 测试最大长度限制
    {
        static constexpr std::array<uint8_t, 10> bytes = {1,2,3,4,5,6,7,8,9,10};
        constexpr auto uchars = std::span(bytes);
        constexpr auto ustr = UCharsView<10>::from_uchars(uchars);
        static_assert(ustr.length() == 10);
    }

    // 超出最大长度的情况
    {
        static constexpr std::array<uint8_t, 15> bytes = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        constexpr auto uchars = std::span(bytes);
        // 应该截断到最大长度
        constexpr auto ustr = UCharsView<10>::from_uchars_bounded(uchars);
        static_assert(ustr.length() == 10);  // 截断到最大长度
    }

    {
        constexpr UCharsView<10> ustr = UCharsView<10>{.p_uchars_ = nullptr, .length_ = 0};
        static_assert(ustr.length() == 0);
    }

    {
        static constexpr std::array<uint8_t, 5> bytes = {0x01, 0x02, 0x03, 0x04, 0x05};
        constexpr auto uchars = std::span(bytes);
        constexpr auto ustr = UCharsView<10>::from_uchars(uchars);
        static_assert(ustr.length() == 5);
    }

    {
        static constexpr std::array<uint8_t, 5> bytes = {0xFF, 0x00, 0x80, 0x7F, 0x01};
        constexpr auto uchars = std::span(bytes);
        constexpr auto ustr = UCharsView<10>::from_uchars(uchars);
        static_assert(ustr.length() == 1);  // 在 null 处停止
        static_assert(ustr[0] == 0xFF);
    }
}


}