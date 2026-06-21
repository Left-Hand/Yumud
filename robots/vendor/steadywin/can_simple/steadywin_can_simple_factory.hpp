#pragma once

#include "steadywin_can_simple_primitive.hpp"
#include "steadywin_can_simple_msgs.hpp"


namespace ymd::robots::steadywin::can_simple{

struct [[nodiscard]] FrameFactoryBackend final{

    struct State{
        can_simple::AxisId axis_id;
    };

    template<typename T>
    static constexpr hal::ClassicCanFrame convert(const State & state, T && msg) noexcept {
        return can_simple::serialize_msg_to_can_frame(state.axis_id, std::forward<T>(msg));
    }
};

template<typename Backend>
struct [[nodiscard]] ApiFacade final{

    using State = typename Backend::State;

    State state;

    constexpr auto clear_errors(this auto && self) noexcept{
        return Backend::convert(self.state, req_msgs::ClearErrors{});
    }

    constexpr auto set_axis_state(
        this auto && self,
        const req_msgs::SetAxisState & msg
    ) noexcept{
        return Backend::convert(self.state, msg);
    }

    constexpr auto set_controller_mode(
        this auto && self,
        const req_msgs::SetControllerMode & msg
    ) noexcept{
        return Backend::convert(self.state, msg);
    }

    constexpr auto set_input_velocity(
        this auto && self,
        const req_msgs::SetInputVelocity & msg
    ) noexcept{
        return Backend::convert(self.state, msg);
    }

    constexpr auto set_input_torque(
        this auto && self,
        const req_msgs::SetInputTorque & msg
    ) noexcept{
        return Backend::convert(self.state, msg);
    }

    constexpr auto set_input_position(
        this auto && self,
        const req_msgs::SetInputPosition & msg
    ) noexcept{
        return Backend::convert(self.state, msg);
    }
};

using FrameFactory = ApiFacade<FrameFactoryBackend>;


}