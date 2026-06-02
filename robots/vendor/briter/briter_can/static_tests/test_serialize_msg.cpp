#include "../briter_can_frame_factory.hpp"

using namespace ymd;
using namespace ymd::robots::briter;


namespace{



[[maybe_unused]] static void test_ser0x01(){
    constexpr auto factory = CanFrameFactory{
        {.node_id = 1}
    };

    {
        static constexpr auto frame = factory.heartbeat_can().to_can_frame();
        static_assert(frame.id_u32() == 1);
        static_assert(frame.length() == 1);
        static_assert(frame.payload_bytes()[0] == 0);
    }

    {
        static constexpr auto frame = factory.set_current({-100}).to_can_frame();
        static_assert(frame.id_u32() == 1);
        static_assert(frame.length() == 3);
        static_assert(frame.payload_bytes()[0] == 0x01);
        static_assert(frame.payload_bytes()[1] == 0xff);
        static_assert(frame.payload_bytes()[2] == 0x9c);
    }

    {
        static constexpr auto frame = factory.set_abs_position({-36000}).to_can_frame();
        static_assert(frame.id_u32() == 1);
        static_assert(frame.length() == 5);
        static_assert(frame.payload_bytes()[0] == 0x04);
        static_assert(frame.payload_bytes()[1] == 0xff);
        static_assert(frame.payload_bytes()[2] == 0xff);
        static_assert(frame.payload_bytes()[3] == 0x73);
        static_assert(frame.payload_bytes()[4] == 0x60);
    }

    {
        static constexpr auto frame = factory.did_find_z_signal().to_can_frame();
        static_assert(frame.id_u32() == 1);
        static_assert(frame.length() == 2);
        static_assert(frame.payload_bytes()[0] == 0x0f);
        static_assert(frame.payload_bytes()[1] == 0x0a);
    }


}


}