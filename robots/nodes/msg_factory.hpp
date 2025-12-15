#pragma once

#include "node_role.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "core/utils/serde.hpp"


namespace ymd{
template<typename E, typename T>
struct command_to_kind{};

template<typename E, E K>
struct kind_to_command{};



template<typename E, typename T>
static constexpr auto command_to_kind_v = command_to_kind<E, T>::KIND;

template<typename E, E K>
using kind_to_command_t = typename kind_to_command<E, K>::type;

};

namespace ymd::robots{



template<typename CommandKind>
static constexpr auto dump_role_and_cmd(const hal::CanStdId id){
    const auto id_u11 = id.to_u11();
    return std::make_tuple(
        std::bit_cast<NodeRole>(uint8_t(id_u11 >> 7)),
        std::bit_cast<CommandKind>(uint8_t(id_u11 & 0x7f))
    );
};


template<typename CommandKind>
static constexpr auto comb_role_and_cmd(const NodeRole role, const CommandKind cmd){
    const auto id_u11 = uint16_t(
        uint16_t(uint16_t(std::bit_cast<uint8_t>(role)) << 7) 
        | uint16_t(std::bit_cast<uint8_t>(cmd) & 0x7f));
    return hal::CanStdId::from_bits(id_u11);
};

template<typename CommandKind>
struct MsgFactory{
    const NodeRole role;

    template<typename T>
    constexpr hal::BxCanFrame operator()(const T cmd) const {
        const auto id = comb_role_and_cmd(role, command_to_kind_v<CommandKind, T>);
        const auto generator = serde::make_serialize_generator<serde::RawLeBytes>(cmd);
        return hal::BxCanFrame(
            id, 
            hal::BxCanPayload::try_from_iter(generator).unwrap()
        );
    };
};


}