#pragma once

#include "ddsm400_api_facade.hpp"

namespace ymd::robots::waveshare::ddsm400{


struct FrameFactoryBackend{
    using Packet = std::array<uint8_t, NUM_PACKET_BYTES>;

    struct State{
        MotorId motor_id;
    };

    template<typename Msg>
    static constexpr Packet convert(const State & state, Msg && msg){
        return transport::serialize_request(state.motor_id, std::forward<Msg>(msg));
    }

};

using FrameFactory = ClientApiFacade<FrameFactoryBackend>;


}