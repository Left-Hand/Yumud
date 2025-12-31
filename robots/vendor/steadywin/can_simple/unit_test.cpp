#include "steadywin_can_simple_primitive.hpp"
#include "steadywin_can_simple_msgs.hpp"

using namespace ymd;
using namespace robots::steadywin;
using namespace robots::steadywin::can_simple;

static constexpr AxisId ZERO_AXIS_ID = AxisId::from_bits(0);


namespace {
//上电校准
[[maybe_unused]] void test1(){
    {
    static constexpr auto frame = serialize_msg_to_can_frame(ZERO_AXIS_ID, req_msgs::SetAxisState{
        .axis_state = AxisState::MotorCalibration
    });
    static_assert(frame.id_u32() == 0x07);
    static_assert(frame.payload_bytes()[0] == 0x04);
    }

    {
        static constexpr auto frame = serialize_msg_to_can_frame(ZERO_AXIS_ID, req_msgs::SetAxisState{
            .axis_state = AxisState::EncoderCalibration
        });
        static_assert(frame.id_u32() == 0x07);
        static_assert(frame.payload_bytes()[0] == 0x07);
    }
}

//速度控制
[[maybe_unused]] void test3(){
    {
    static constexpr auto frame = serialize_msg_to_can_frame(ZERO_AXIS_ID, 
        req_msgs::SetControllerMode{
            .loop_mode = LoopMode::VelocityLoop,
            .input_mode = InputMode::VelocityRamp
        }
    );
    static_assert(frame.id_u32() == 0x0b);
    static_assert(frame.payload_bytes()[0] == 0x02);
    static_assert(frame.payload_bytes()[4] == 0x02);
    }
    {
        static constexpr auto frame = serialize_msg_to_can_frame(ZERO_AXIS_ID, 
            req_msgs::SetAxisState{
                .axis_state = AxisState::ClosedLoopControl,
            }
        );
        static_assert(frame.id_u32() == 0x07);
        static_assert(frame.payload_bytes()[0] == 0x08);
    }
    {
        static constexpr auto frame = serialize_msg_to_can_frame(ZERO_AXIS_ID, 
            req_msgs::SetInputVelocity{
                .vel_ff = 10,
                .torque_ff = 0
            }
        );
        static_assert(frame.id_u32() == 0x0d);
        static_assert(frame.payload_bytes()[0] == 0x00);
        static_assert(frame.payload_bytes()[1] == 0x00);
        static_assert(frame.payload_bytes()[2] == 0x20);
        static_assert(frame.payload_bytes()[3] == 0x41);
    }
}

//位置控制
[[maybe_unused]] void test6(){
    {
        static constexpr auto frame = serialize_msg_to_can_frame(ZERO_AXIS_ID, 
            req_msgs::SetControllerMode{
                .loop_mode = LoopMode::PositionLoop,
                .input_mode = InputMode::PositionFilter
            }
        );
        static_assert(frame.id_u32() == 0x0b);
        static_assert(frame.payload_bytes()[0] == 0x03);
        static_assert(frame.payload_bytes()[1] == 0x00);
        static_assert(frame.payload_bytes()[2] == 0x00);
        static_assert(frame.payload_bytes()[3] == 0x00);
        static_assert(frame.payload_bytes()[4] == 0x03);
    }

    {
        static constexpr auto frame = serialize_msg_to_can_frame(ZERO_AXIS_ID, 
            req_msgs::SetAxisState{
                .axis_state = AxisState::ClosedLoopControl,
            }
        );
        static_assert(frame.id_u32() == 0x07);
        static_assert(frame.payload_bytes()[0] == 0x08);
    }

    {
        static constexpr auto frame = serialize_msg_to_can_frame(ZERO_AXIS_ID, 
            req_msgs::SetInputPosition{
                .input_position = 2.2,
                .vel_ff = 0,
                .torque_ff = 0,
            }
        );
        static_assert(frame.dlc().length() == 8);
        static_assert(frame.id_u32() == 0x0c);
        static_assert(frame.payload_bytes()[0] == 0xcd);
        static_assert(frame.payload_bytes()[1] == 0xcc);
        static_assert(frame.payload_bytes()[2] == 0x0c);
        static_assert(frame.payload_bytes()[3] == 0x40);
    }
}

}