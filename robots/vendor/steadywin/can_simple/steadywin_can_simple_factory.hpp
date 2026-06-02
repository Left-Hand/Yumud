#pragma once

#include "steadywin_can_simple_primitive.hpp"
#include "steadywin_can_simple_msgs.hpp"

namespace ymd::robots::steadywin::can_simple{

using namespace primitive;


struct [[nodiscard]] FrameFactoryBackend final{
    template<typename T>
    using proceed_type_t = hal::ClassicCanFrame;

    template<typename F, typename T>
    static constexpr hal::ClassicCanFrame convert(const F & self, T && msg) noexcept {
        return can_simple::serialize_msg_to_can_frame(self.axis_id, std::forward<T>(msg));
    }
};

template<typename Backend>
struct [[nodiscard]] ApiFacade final{

    can_simple::AxisId axis_id;

    constexpr auto clear_errors()  const {
        return Backend::convert(*this, req_msgs::ClearErrors{});
    }

    constexpr auto set_axis_state(
        const req_msgs::SetAxisState & msg
    )  const {
        return Backend::convert(*this, msg);
    }

    constexpr auto set_controller_mode(
        const req_msgs::SetControllerMode & msg
    )  const {
        return Backend::convert(*this, msg);
    }

    constexpr auto set_input_velocity(
        const req_msgs::SetInputVelocity & msg
    )  const {
        return Backend::convert(*this, msg);
    }

    constexpr auto set_input_torque(
        const req_msgs::SetInputTorque & msg
    )  const {
        return Backend::convert(*this, msg);
    }

    constexpr auto set_input_position(
        const req_msgs::SetInputPosition & msg
    )  const {
        return Backend::convert(*this, msg);
    }
};

using FrameFactory = ApiFacade<FrameFactoryBackend>;


}