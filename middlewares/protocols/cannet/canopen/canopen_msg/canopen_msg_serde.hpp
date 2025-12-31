#pragma once

#include "flex_assert.hpp"
#include "primitive/can/bxcan_frame.hpp"

namespace ymd::canopen::msg_serde{
using namespace primitive;

template<typename T>
struct MsgSerde;


template<typename T>
static constexpr auto to_can_frame(const T& self){
    return MsgSerde<T>::to_can_frame(self);
}

template<typename T, VerifyLevel verify_level = VerifyLevel::Propagate>
static constexpr auto from_can_frame(const hal::BxCanFrame & frame){
    return MsgSerde<T>::template from_can_frame<verify_level>(frame);
}
}