#include "../u11x16.hpp"

using namespace ymd;;

namespace {
[[maybe_unused]] void test_u11x16() {
    {
        static constexpr uint16_t BASE = 0x80;
        static constexpr auto elements = []{
            U11X16Owned temp{};
            std::fill(temp.bytes.begin(), temp.bytes.end(), 0);
            for (size_t i = 0; i < 16; ++i) {
                temp[i] = BASE + i;
            }
            return temp;
        }();

        // 验证所有16个通道
        static_assert(elements[0] == BASE + 0);
        static_assert(elements[1] == BASE + 1);
        static_assert(elements[2] == BASE + 2);
        static_assert(elements[3] == BASE + 3);
        static_assert(elements[4] == BASE + 4);
        static_assert(elements[5] == BASE + 5);
        static_assert(elements[6] == BASE + 6);
        static_assert(elements[7] == BASE + 7);
        static_assert(elements[8] == BASE + 8);
        static_assert(elements[9] == BASE + 9);
        static_assert(elements[10] == BASE + 10);
        static_assert(elements[11] == BASE + 11);
        static_assert(elements[12] == BASE + 12);
        static_assert(elements[13] == BASE + 13);
        static_assert(elements[14] == BASE + 14);
        static_assert(elements[15] == BASE + 15);
    }
}


}