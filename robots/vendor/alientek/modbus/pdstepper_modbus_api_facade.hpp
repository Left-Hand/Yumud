#pragma once

#include "../pdstepper_primitive.hpp"
#include "middlewares/protocols/modbus/modbus_msgs.hpp"
#include "middlewares/protocols/modbus/modbus_serialize.hpp"
#include "core/math/float/fp32.hpp"

namespace ymd::robots::pdstepper{

template<typename Backend>
struct [[nodiscard]] ClientApiFacade final{
    using State = typename Backend::State;

    State state;

    constexpr auto calibrate_encoder(this auto && self) {
        return Backend::write_16(self.state, Command::CalibrateEncoder, 0x01);
    }

    constexpr auto reset(this auto && self) {
        return Backend::write_16(self.state, Command::Reset, 0x01);
    }

    constexpr auto restore_factory(this auto && self) {
        return Backend::write_16(self.state, Command::RestoreFactory, 0x01);
    }

    constexpr auto get_swhw_version(this auto && self) {
        return Backend::read_16(self.state, Command::GetSoftHardVer, 0x01);
    }

    constexpr auto get_flux(this auto && self) {
        return Backend::read_16(self.state, Command::GetFlux, 0x02);
    }


    // 5.3.3 读取相电阻和相电感指令
    constexpr auto get_phase_resind(this auto && self) {
        return Backend::read_16(self.state, Command::GetPhaseResInd, 0x04);
    }


    // 5.3.4 读取相电流指令
    constexpr auto get_phase_current(this auto && self) {
        return Backend::read_16(self.state, Command::GetPhaseCurrent, 0x04);
    }

    // 5.3.5 读取总线电压指令
    constexpr auto get_busbar_voltage(this auto && self) {
        return Backend::read_16(self.state, Command::GetBusbarVoltage, 0x04);
    }

    // 5.3.22 设置位置环 PID 参数指令
    constexpr auto set_position_pid_paraments(
        this auto && self, 
        std::array<uint32_t, 3> paraments
    ) {
        return Backend::write_32arr(self.state, Command::SetPositionPidParaments, std::span(paraments));
    }

};


}