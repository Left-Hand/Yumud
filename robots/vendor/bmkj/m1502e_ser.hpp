#pragma once

#include "m1502e_msgs.hpp"
namespace ymd::robots::bmkj::m1502e{

template<typename T>
static constexpr hal::ClassicCanFrame serialize_msg(const T & msg){
    std::array<uint8_t, 8> buf;
    msg.fill_bytes(buf);
    return hal::ClassicCanFrame::from_parts(
        hal::CanStdId::from_u11(T::NUM_CANID),
        hal::ClassicCanPayload::from_u8x8(buf)
    );
}

struct [[nodiscard]] FrameFactory final{
public:
    constexpr hal::ClassicCanFrame set_low_quad_motor_setpoint(
        const req_msgs::SetLowQuadMotorSetpoint & msg
    ) const{
        return serialize_msg(msg);
    }

    constexpr hal::ClassicCanFrame set_loop_mode(const req_msgs::SetLoopMode & msg) const {
        return serialize_msg(msg);
    }

    constexpr hal::ClassicCanFrame set_feedback_strategy(
        const req_msgs::SetFeedbackStrategy & msg
    ) const{
        return serialize_msg(msg);
    }

    constexpr hal::ClassicCanFrame set_motor_id(const req_msgs::SetMotorId & msg) const{
        return serialize_msg(msg);
    }

    constexpr hal::ClassicCanFrame query_items(const req_msgs::QueryItems & msg) const{
        return serialize_msg(msg);
    }

    constexpr hal::ClassicCanFrame query_firmware_version(const req_msgs::QueryFirmwareVersion & msg) const{
        return serialize_msg(msg);
    }
private:
    template<typename T>
    constexpr hal::ClassicCanFrame serialize(const T & msg) const {
        return serialize_msg(msg);
    }
};
}