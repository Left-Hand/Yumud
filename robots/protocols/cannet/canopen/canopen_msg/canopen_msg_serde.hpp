#pragma once

#include "flex_assert.hpp"
#include "primitive/can/bxcan_frame.hpp"

namespace ymd::canopen::msg_serde{
using namespace primitive;

template<typename T>
struct MsgSerde;


template<typename T>
static constexpr auto to_canmsg(const T& self){
    return MsgSerde<T>::to_canmsg(self);
}

template<typename T, VerifyLevel verify_level = VerifyLevel::Propagate>
static constexpr auto from_canmsg(const hal::CanClassicFrame & frame){
    return MsgSerde<T>::template from_canmsg<verify_level>(frame);
}
}