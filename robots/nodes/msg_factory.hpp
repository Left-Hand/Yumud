#pragma once

#include "node_role.hpp"
#include "hal/bus/can/can_msg.hpp"
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
        std::bit_cast<NodeRole>(uint8_t(id_u11 & 0x7f)),
        std::bit_cast<CommandKind>(uint8_t(id_u11 >> 7))
    );
};


template<typename CommandKind>
static constexpr auto comb_role_and_cmd(const NodeRole role, const CommandKind cmd){
    const auto id_u11 = uint16_t(
        std::bit_cast<uint8_t>(role) 
        | (std::bit_cast<uint8_t>(cmd) << 7));
    return hal::CanStdId(id_u11);
};

template<typename CommandKind>
struct MsgFactory{
    const NodeRole role;

    template<typename T>
    constexpr hal::CanMsg operator()(const T cmd){
        const auto id = comb_role_and_cmd(role, command_to_kind_v<CommandKind, T>);
        const auto iter = serde::make_serialize_iter<serde::RawBytes>(cmd);
        return hal::CanMsg::from_iter(id, iter).unwrap();
    };
};


}