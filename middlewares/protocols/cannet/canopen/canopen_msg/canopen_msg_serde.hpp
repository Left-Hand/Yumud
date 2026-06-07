#pragma once

#include "flex_assert.hpp"
#include "primitive/can/can_frame.hpp"

namespace ymd::canopen::msg_serde{
using namespace primitive;

template<typename T>
struct MsgSerde;


template<typename T>
requires requires(T t) { t.to_can_frame(); }
static constexpr auto to_can_frame(const T& self){
    return self.to_can_frame();
}

template<typename T, VerifyLevel VERIFY_LEVEL = VerifyLevel::Propagate>
requires requires(const CanFrame& frame) { T::template from_can_frame<VERIFY_LEVEL>(frame); }
static constexpr auto from_can_frame(const CanFrame & frame){
    return T::template from_can_frame<VERIFY_LEVEL>(frame);
}


}