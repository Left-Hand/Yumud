#include "fsi6x_primitive.hpp"


using namespace ymd;
using namespace ymd::robots::flyskytech::fsi6x;


namespace{

[[maybe_unused]] void test_u11x16() {
    {
        static constexpr uint16_t BASE = 0x80;
        static constexpr auto channels = []{
            utils::U11X16 temp{};
            std::fill(temp.bytes.begin(), temp.bytes.end(), 0);
            for (size_t i = 0; i < 16; ++i) {
                temp[i] = BASE + i;
            }
            return temp;
        }();

        // 验证所有16个通道
        static_assert(channels[0] == BASE + 0);
        static_assert(channels[1] == BASE + 1);
        static_assert(channels[2] == BASE + 2);
        static_assert(channels[3] == BASE + 3);
        static_assert(channels[4] == BASE + 4);
        static_assert(channels[5] == BASE + 5);
        static_assert(channels[6] == BASE + 6);
        static_assert(channels[7] == BASE + 7);
        static_assert(channels[8] == BASE + 8);
        static_assert(channels[9] == BASE + 9);
        static_assert(channels[10] == BASE + 10);
        static_assert(channels[11] == BASE + 11);
        static_assert(channels[12] == BASE + 12);
        static_assert(channels[13] == BASE + 13);
        static_assert(channels[14] == BASE + 14);
        static_assert(channels[15] == BASE + 15);
    }
}


}