#pragma once

#include "cybergear_prelude.hpp"


namespace ymd::robots::cybergear{

struct MitParams{
    TorqueCode torque_code;
    RadCode rad_code;
    OmegaCode omega_code; 
    KpCode kp_code; 
    KdCode kd_code;
};

class [[nodiscard]] FrameFactory final{
public:
    struct State{
        uint8_t host_id;
        uint8_t node_id;
    };

    State state;

    constexpr hal::ClassicCanFrame request_mcu_id(this auto && self) noexcept {

        return hal::ClassicCanFrame::from_empty_data(
            CgId::from_parts(Command::GetDeviceId, self.state.host_id, self.state.node_id).to_extid()
        );
    }

    constexpr hal::ClassicCanFrame ctrl(this auto && self, const MitParams & params) noexcept {
        TxContext tx_context;

        tx_context.cmd_rad() = params.rad_code; 
        tx_context.cmd_omega() = params.omega_code;
        tx_context.cmd_kd() = params.kd_code;
        tx_context.cmd_kp() = params.kp_code;

        const auto ext_id = CgId::from_parts(
            Command::SendCtrl1, 
            params.torque_code.to_bits(), 
            self.state.node_id).to_extid();

        return hal::ClassicCanFrame::from_parts(
            ext_id,
            tx_context.to_can_payload()
        );
    }

    constexpr hal::ClassicCanFrame enable(this auto && self) noexcept {
        return hal::ClassicCanFrame::from_parts(
            CgId::from_parts(Command::EnableMotor, self.state.host_id, self.state.node_id).to_extid(),
            hal::ClassicCanPayload::from_u64(0)
        );
    }

    constexpr hal::ClassicCanFrame disable(this auto && self, const bool clear_fault) noexcept {

        uint64_t data_u64 = 0;
        data_u64 |= (clear_fault) ? 1u << 0 : 0;

        // 正常运行时，data区需清0；
        // byte[0]=1 时：清故障；
        return hal::ClassicCanFrame::from_parts(
            CgId::from_parts(Command::DisableMotor, self.state.host_id, self.state.node_id).to_extid(), 
            hal::ClassicCanPayload::from_u64(data_u64)
        );
    }


    constexpr hal::ClassicCanFrame set_now_as_machine_home(this auto && self) noexcept {

        const auto ext_id = CgId::from_parts(Command::SetMachineHome, 
            self.state.host_id, self.state.node_id).to_extid();

        return hal::ClassicCanFrame::from_parts(
            ext_id, 
            hal::ClassicCanPayload::from_u64(1u)
        );
    }

    constexpr hal::ClassicCanFrame set_motor_node_id(this auto && self, const uint8_t new_node_id) noexcept {
        const auto ext_id = CgId::from_parts(
            Command::SetCanId, 
            self.state.host_id | (new_node_id << 8), 
            self.state.node_id
        ).to_extid();

        return hal::ClassicCanFrame::from_parts(
            ext_id, 
            hal::ClassicCanPayload::from_u64(0)
        );
    }

    constexpr hal::ClassicCanFrame request_read_para(this auto && self, const uint16_t idx) noexcept {
        static constexpr size_t LENGTH = 8;
        std::array<uint8_t, LENGTH> buffer = {
            static_cast<uint8_t>(idx & 0xFF),
            static_cast<uint8_t>(idx >> 8),
            0, 0, 
            0, 0, 0, 0
        };

        return hal::ClassicCanFrame::from_parts(
            CgId::from_parts(Command::ReadParam, self.state.host_id, self.state.node_id).to_extid(), 
            hal::ClassicCanPayload::from_u8x8(std::move(buffer))
        );
    }

    constexpr hal::ClassicCanFrame request_write_para(
        this auto && self,
        const uint16_t idx, 
        const uint32_t param_bits
    ) noexcept {

        static constexpr size_t LENGTH = 8;
        std::array<uint8_t, LENGTH> buffer = {
            static_cast<uint8_t>(idx & 0xFF),
            static_cast<uint8_t>(idx >> 8),
            0, 0, 
            static_cast<uint8_t>(param_bits & 0xff),
            static_cast<uint8_t>((param_bits >> 8) & 0xff),
            static_cast<uint8_t>((param_bits >> 16) & 0xff),
            static_cast<uint8_t>((param_bits >> 24) & 0xff)
        };


        return hal::ClassicCanFrame::from_parts(
            CgId::from_parts(Command::WriteParam, self.state.host_id, self.state.node_id).to_extid(), 
            hal::ClassicCanPayload::from_u8x8(std::move(buffer))
        );
    }
};


}