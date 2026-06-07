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

    constexpr auto heartbeat_modbus(this auto && self, uint16_t rolling_counter_value) noexcept {
        return self.state.write_16(ItemId::Heartbeat, rolling_counter_value);
    } 

    constexpr auto heartbeat_can(this auto && self) noexcept {
        return self.state.write_0(ItemId::Heartbeat);
    } 

    constexpr auto set_current(this auto && self, const CurrentCode code) noexcept {
        return self.state.write_16(ItemId::SetCurrent, std::bit_cast<uint16_t>(code));
    } 

    constexpr auto set_abs_position(this auto && self, const PositionCode code) noexcept {
        return self.state.write_32(ItemId::SetAbsPosition, std::bit_cast<uint32_t>(code));
    } 

    constexpr auto set_erpm(this auto && self, const ErpmCode code) noexcept {
        return self.state.write_32(ItemId::SetSpeed, std::bit_cast<uint32_t>(code));
    } 

    constexpr auto set_duty(this auto && self, const DutyCode code) noexcept {
        return self.state.write_16(ItemId::SetDuty, std::bit_cast<uint16_t>(code));
    }

    constexpr auto get_duty(this auto && self) noexcept {
        return self.state.read_16(ItemId::RealtimeDuty);
    }

    constexpr auto set_ctrl_mode(this auto && self, const ControlMode mode) noexcept {
        return self.state.write_16(ItemId::CtrlMode, std::bit_cast<uint16_t>(mode));
    }

    constexpr auto get_homming_status(this auto && self) noexcept {
        return self.state.read_16(ItemId::HommingStatus);
    }

    constexpr auto did_find_z_signal(this auto && self) noexcept {
        return self.state.read_16(ItemId::FindZSignal);
    }

    constexpr auto get_position(this auto && self) noexcept {
        return self.state.read_32(ItemId::RealtimePosition);
    }


private:
};
}