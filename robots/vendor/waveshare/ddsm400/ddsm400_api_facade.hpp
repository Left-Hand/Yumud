#pragma once

#include "ddsm400_msgs.hpp"
#include "ddsm400_transport.hpp"

namespace ymd::robots::waveshare::ddsm400{


template<typename Backend>
struct [[nodiscard]] ClientApiFacade{

    using State = Backend::State;

    State state;

    constexpr auto set_target(this auto && self, const req_msgs::SetTarget & msg) noexcept {
        return Backend::convert(self.state, msg);
    }

    constexpr auto get_journey(this auto && self) noexcept {
        return Backend::convert(self.state, req_msgs::GetJourney{});
    }

    constexpr auto set_loop_mode(this auto && self, const req_msgs::SetLoopMode & msg) noexcept {
        return Backend::convert(self.state, msg);
    }

    constexpr auto set_motor_id(this auto && self, const req_msgs::SetMotorId & msg) noexcept {
        return Backend::convert(self.state, msg);
    }

    constexpr auto get_loop_mode(this auto && self) noexcept {
        return Backend::convert(self.state, req_msgs::GetLoopMode{});
    }

private:


};



}