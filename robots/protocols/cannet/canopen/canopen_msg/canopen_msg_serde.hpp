#pragma once

#include "flex_assert.hpp"
#include "primitive/can/can_msg.hpp"

namespace ymd::canopen::msg_serde{
using namespace primitive;

template<typename T>
struct MsgSerde;


template<typename T>
static constexpr auto to_canmsg(const T& self){
    return MsgSerde<T>::to_canmsg(self);
}

template<typename T, AssertLevel assert_level = AssertLevel::Propagate>
static constexpr auto from_canmsg(const hal::CanClassicMsg & msg){
    return MsgSerde<T>::template from_canmsg<assert_level>(msg);
}
}