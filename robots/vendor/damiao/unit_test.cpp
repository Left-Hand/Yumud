#include "damiao_factory.hpp"


using namespace ymd::robots::damiao;
namespace{

[[maybe_unused]] void test_ser_msgs(){
    static constexpr uint32_t NUM_MOTOR_ID = 0x03;

    constexpr auto factory = FrameFactory{
        .motor_id = NodeId(NUM_MOTOR_ID)
    };

    {
        constexpr auto frame = factory.disable();
        static_assert(frame.id_u32() == NUM_MOTOR_ID);
        static_assert(frame.length() == 8);
        static_assert(frame.payload()[0] == 0xff);
        static_assert(frame.payload()[1] == 0xff);
        static_assert(frame.payload()[2] == 0xff);
        static_assert(frame.payload()[3] == 0xff);
        static_assert(frame.payload()[4] == 0xff);
        static_assert(frame.payload()[5] == 0xff);
        static_assert(frame.payload()[6] == 0xff);
        static_assert(frame.payload()[7] == 0xfd);
    }

    {
        constexpr auto frame = factory.clear_error();
        static_assert(frame.id_u32() == NUM_MOTOR_ID);
        static_assert(frame.length() == 8);
        static_assert(frame.payload()[0] == 0xff);
        static_assert(frame.payload()[1] == 0xff);
        static_assert(frame.payload()[2] == 0xff);
        static_assert(frame.payload()[3] == 0xff);
        static_assert(frame.payload()[4] == 0xff);
        static_assert(frame.payload()[5] == 0xff);
        static_assert(frame.payload()[6] == 0xff);
        static_assert(frame.payload()[7] == 0xfb);
    }

    {
        constexpr auto frame = factory.posvel_control({.q = 2.0f, .dq = 1.0f});
        static_assert(frame.id_u32() == 0x100 + NUM_MOTOR_ID);
        static_assert(frame.length() == 8);
    }

    {
        constexpr auto frame = factory.vel_control({.dq = 1.0f});
        static_assert(frame.id_u32() == 0x200 + NUM_MOTOR_ID);
        static_assert(frame.length() == 4);
    }


}

}
