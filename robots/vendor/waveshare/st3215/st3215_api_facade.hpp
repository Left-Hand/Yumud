#pragma once

#include "st3215_primitive.hpp"
#include "st3215_utils.hpp"

namespace ymd::robots::waveshare::st3215{

struct ErasedPacket{

};


struct FrameFactoryBackend{
    struct State{
        const ServoId id;
    };

    template<typename Msg>
    static constexpr ErasedPacket convert(const State & state, Msg && msg){
        return ErasedPacket{};
    }
};


template<typename Backend>
struct [[nodiscard]] ClientApiFacade final{

    using State = typename Backend::State;


    [[nodiscard]] constexpr auto ping(this auto && self) noexcept{
        return Backend::convert(self.state, ins_msgs::Ping{});
    }

    // [[nodiscard]] constexpr auto write_data(
    //     const uint8_t addr, 
    //     const std::span<const uint8_t, N> data
    // ){
    //     return Backend::convert(self.state, ins_msgs::WriteData{.addr = addr, .val});
    // }

};


}