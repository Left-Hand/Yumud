#pragma once

#include <bit>
#include "briter_primitive.hpp"

namespace ymd::robots::briter{

//TODO 添加所有支持的方法
template<typename Backend>
struct [[nodiscard]] ClientApiFacade final{

    using State = typename Backend::State;
    using ItemId = typename Backend::ItemId;

    State state;

    constexpr auto heartbeat_modbus(uint16_t rolling_counter_value) const {
        return state.write16(ItemId::Heartbeat, rolling_counter_value);
    } 

    constexpr auto heartbeat_can() const {
        return state.write0(ItemId::Heartbeat);
    } 

    constexpr auto set_current(const CurrentCode code) const {
        return state.write16(ItemId::SetCurrent, std::bit_cast<uint16_t>(code));
    } 

    constexpr auto set_abs_position(const PositionCode code) const {
        return state.write32(ItemId::SetAbsPos, std::bit_cast<uint32_t>(code));
    } 

    constexpr auto set_erpm(const ErpmCode code) const {
        return state.write32(ItemId::SetSpeed, std::bit_cast<uint32_t>(code));
    } 

    constexpr auto set_duty(const DutyCode code) const{
        return state.write16(ItemId::SetDuty, std::bit_cast<uint16_t>(code));
    }

    constexpr auto get_duty() const{
        return state.read16(ItemId::RealtimeDuty);
    }

    constexpr auto set_ctrl_mode(const ControlMode mode) const {
        return state.write16(ItemId::CtrlMode, std::bit_cast<uint16_t>(mode));
    }

    constexpr auto get_homming_status() const {
        return state.read16(ItemId::HommingStatus);
    }

    constexpr auto did_find_z_signal() const {
        return state.read16(ItemId::FindZSignal);
    }

    constexpr auto get_position() const {
        return state.read32(ItemId::RealtimePos);
    }


private:
};
}