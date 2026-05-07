#include "crsf_extended_msgs.hpp"

using namespace ymd;
using namespace ymd::crsf;

// https://www.zhihu.com/question/4436519767/answer/34255755305

namespace {

[[maybe_unused]] void test_ser_msg(){
    {
        [[maybe_unused]] static constexpr auto msg = msgs::ParameterSettingsRead{
            .parameter_number = 0x11,
            .parameter_chunk_number = 0x45
        };

        auto fn = []{
            std::array<uint8_t, 5> buf{0xff, 0xff, 0xff, 0xff, 0xff};
            auto receiver = SerializeReceiver{std::span(buf), 0};
            msg.serialize(receiver).unwrap();
            return std::make_tuple(buf, receiver);
        };

        static constexpr auto bytes = std::get<0>(fn());
        static_assert(bytes[0] == 0x11);
        static_assert(bytes[1] == 0x45);
        static_assert(bytes[2] == 0xff);
        static_assert(bytes[3] == 0xff);
        static_assert(bytes[4] == 0xff);

    }
}




}