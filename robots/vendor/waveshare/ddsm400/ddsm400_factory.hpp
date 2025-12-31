#pragma once

#include "ddsm400_msgs.hpp"
#include "ddsm400_transport.hpp"

namespace ymd::robots::waveshare::ddsm400{

using namespace primitive;


struct [[nodiscard]] FrameFactory{
    using Packet = std::array<uint8_t, NUM_PACKET_BYTES>;

    MotorId motor_id;

    constexpr Packet set_target(const req_msgs::SetTarget & msg) const {
        return serialize(msg);
    }

    constexpr Packet get_journey() const {
        return serialize(req_msgs::GetJourney{});
    }

    constexpr Packet set_loop_mode(const req_msgs::SetLoopMode & msg) const {
        return serialize(msg);
    }

    constexpr Packet set_motor_id(const req_msgs::SetMotorId & msg) const {
        return serialize(msg);
    }

    constexpr Packet get_loop_mode() const {
        return serialize(req_msgs::GetLoopMode{});
    }

private:

    template<typename T>
    constexpr Packet serialize(T && msg) const {
        return transport::serialize_request(motor_id, std::forward<T>(msg));
    }
};


}