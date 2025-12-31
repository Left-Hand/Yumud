#pragma once

#include "steadywin_can_simple_primitive.hpp"
#include "steadywin_can_simple_msgs.hpp"

namespace ymd::robots::steadywin::can_simple{

using namespace primitive;




struct [[nodiscard]] FrameFactory{

    can_simple::AxisId axis_id;


    constexpr hal::BxCanFrame clear_errors()  const {
        return serialize(req_msgs::ClearErrors{});
    }

    constexpr hal::BxCanFrame set_axis_state(const req_msgs::SetAxisState msg)  const {
        return serialize(msg);
    }

    constexpr hal::BxCanFrame set_controller_mode(const req_msgs::SetControllerMode msg)  const {
        return serialize(msg);
    }

    constexpr hal::BxCanFrame set_input_velocity(const req_msgs::SetInputVelocity msg)  const {
        return serialize(msg);
    }

    constexpr hal::BxCanFrame set_input_torque(const req_msgs::SetInputTorque msg)  const {
        return serialize(msg);
    }

    constexpr hal::BxCanFrame set_input_position(const req_msgs::SetInputPosition msg)  const {
        return serialize(msg);
    }

private:

    template<typename T>
    constexpr hal::BxCanFrame serialize(T && msg) const {
        return can_simple::serialize_msg_to_can_frame(axis_id, std::forward<T>(msg));
    }
};


}