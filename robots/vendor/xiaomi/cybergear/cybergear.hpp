#pragma once

#include "cybergear_primitive.hpp"


namespace ymd::robots::cybergear{


class CyberGearFactory{
public:
    struct MitParams{
        TorqueCode torque_code;
        RadCode rad_code;
        OmegaCode omega_code; 
        KpCode kp_code; 
        KdCode kd_code;
    };


    uint8_t host_id;
    uint8_t node_id;



    constexpr hal::ClassicCanFrame request_mcu_id(){
        const auto extid = CgId::from_parts(
            cybergear::Command::GET_DEVICE_ID, host_id, node_id).to_extid();

        return hal::ClassicCanFrame::from_empty_data(
            extid
        );
    }

    constexpr hal::ClassicCanFrame ctrl(const MitParams & params){
        TxContext tx_context = {};

        tx_context.cmd_rad() = params.rad_code; 
        tx_context.cmd_omega() = params.omega_code;
        tx_context.cmd_kd() = params.kd_code;
        tx_context.cmd_kp() = params.kp_code;

        const auto extid = CgId::from_parts(
            cybergear::Command::SEND_CTRL1, 
            params.torque_code.to_bits(), 
            node_id).to_extid();

        return hal::ClassicCanFrame::from_parts(
            extid,
            tx_context.to_can_payload()
        );
    }

    constexpr hal::ClassicCanFrame enable(){
        const auto extid = CgId::from_parts(cybergear::Command::EN_MOT, host_id, node_id).to_extid();
        return hal::ClassicCanFrame::from_parts(
            extid, 
            hal::ClassicCanPayload::from_u64(0)
        );
    }

    constexpr hal::ClassicCanFrame disable(const bool clear_fault){

        uint64_t data_u64 = 0;
        data_u64 |= (clear_fault) ? 1u << 0 : 0;

        // 正常运行时，data区需清0；
        // byte[0]=1 时：清故障；
        return hal::ClassicCanFrame::from_parts(
            CgId::from_parts(cybergear::Command::DISEN_MOT, host_id, node_id).to_extid(), 
            hal::ClassicCanPayload::from_u64(data_u64)
        );
    }


    constexpr hal::ClassicCanFrame set_now_as_machine_home(){
        static constexpr size_t LENGTH = 8;
        static constexpr std::array<uint8_t, LENGTH> buffer = {
            1,
            0, 0, 0, 
            0, 0, 0, 0
        };

        const auto extid = CgId::from_parts(cybergear::Command::SET_MACHINE_HOME, 
            host_id, node_id).to_extid();

        return hal::ClassicCanFrame::from_parts(
            extid, 
            hal::ClassicCanPayload::from_u8x8(buffer)
        );
    }

    constexpr hal::ClassicCanFrame set_motor_nodeid(const uint8_t new_node_id){
        const auto extid = CgId::from_parts(
            cybergear::Command::SET_CAN_ID, 
            host_id | (new_node_id << 8), 
            node_id
        ).to_extid();

        return hal::ClassicCanFrame::from_parts(
            extid, 
            hal::ClassicCanPayload::from_u64(0)
        );
    }

    constexpr hal::ClassicCanFrame request_read_para(const uint16_t idx){
        static constexpr size_t LENGTH = 8;
        std::array<uint8_t, LENGTH> buffer = {
            static_cast<uint8_t>(idx & 0xFF),
            static_cast<uint8_t>(idx >> 8),
            0, 0, 
            0, 0, 0, 0
        };

        return hal::ClassicCanFrame::from_parts(
            CgId::from_parts(cybergear::Command::READ_PARA, host_id, node_id).to_extid(), 
            hal::ClassicCanPayload::from_u8x8(std::move(buffer))
        );
    }

    constexpr hal::ClassicCanFrame request_write_para(
        const uint16_t idx, 
        const uint32_t param_bits
    ){

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
            CgId::from_parts(cybergear::Command::WRITE_PARA, host_id, node_id).to_extid(), 
            hal::ClassicCanPayload::from_u8x8(std::move(buffer))
        );
    }
};


}