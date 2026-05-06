#pragma once

#include "damiao_msgs.hpp"

namespace ymd::robots::damiao{


struct FrameFactory{

    const NodeId motor_id;



    //4.2 mit 控制
    constexpr hal::ClassicCanFrame mit_control(const MitParams& mit_param) const noexcept {
        return hal::ClassicCanFrame::from_parts(NO_BASE + motor_id, mit_param.to_can_payload());
    }


    //4.3
    constexpr hal::ClassicCanFrame posvel_control(const PosVelParam& posvel_param) const noexcept {
        // pos vel mode needs extra 0x100
        return hal::ClassicCanFrame::from_parts(POS_VEL_MODE_BASE + motor_id, posvel_param.to_can_payload());
    }

    //4.4
    constexpr hal::ClassicCanFrame vel_control(const VelParam& vel_param) const noexcept {
        // pos mode needs extra 0x200
        return hal::ClassicCanFrame::from_parts(VEL_ONLY_MODE_BASE + motor_id, vel_param.to_can_payload());
    }


    constexpr hal::ClassicCanFrame posforce_control(const PosForceParam& posforce_param) {
        // pos force mode needs extra 0x300
        return hal::ClassicCanFrame::from_parts(POS_FORCE_MODE_BASE + motor_id, posforce_param.to_can_payload());
    }


    // 7.2.1 使能、失能、保存零点、清除错误
    // 电机需要发送使能命令后，电机LED由红变绿之后才可以进行控制，无论使用哪种模
    // 式，使能电机的命令都是一样的

    //4.5
    constexpr hal::ClassicCanFrame enable() const noexcept {
        return hal::ClassicCanFrame::from_parts(NO_BASE + motor_id, pack_0xff_and_tail(0xFC));
    }

    //4.6
    constexpr hal::ClassicCanFrame disable() const noexcept {
        return hal::ClassicCanFrame::from_parts(NO_BASE + motor_id, pack_0xff_and_tail(0xFD));
    }

    //4.7 保存零点
    constexpr hal::ClassicCanFrame set_zero() const noexcept {
        return hal::ClassicCanFrame::from_parts(NO_BASE + motor_id, pack_0xff_and_tail(0xFE));
    }

    //4.8 清除错误
    constexpr hal::ClassicCanFrame clear_error() const noexcept {
        return hal::ClassicCanFrame::from_parts(NO_BASE + motor_id, pack_0xff_and_tail(0xFC));
    }

    template<typename T>
    requires (sizeof(T) == 4)
    constexpr hal::ClassicCanFrame write_param(const uint8_t reg_addr, const T param){
        const auto && param_bytes = std::bit_cast<std::array<uint8_t, 4>>(param);
        return pack_write_param(motor_id, reg_addr, param_bytes); 
    }

    constexpr hal::ClassicCanFrame query_param(uint8_t reg_addr) const noexcept {
        return hal::ClassicCanFrame::from_parts(NMT_CAN_FRAME_ID, pack_query_param_data(motor_id, reg_addr));
    }

private:
    struct [[nodiscard]] IdBase final{
        uint16_t count;

        constexpr hal::CanStdId  operator +(const uint8_t nodeid) const noexcept {
            return hal::CanStdId::from_u11(count + nodeid);
        }
    };

    static constexpr IdBase NO_BASE = {0x000};
    static constexpr IdBase POS_VEL_MODE_BASE = {0x100};
    static constexpr IdBase VEL_ONLY_MODE_BASE = {0x200};
    static constexpr IdBase POS_FORCE_MODE_BASE = {0x300};
    static constexpr hal::CanStdId NMT_CAN_FRAME_ID = hal::CanStdId::from_u11(0x7FF);

    static constexpr hal::ClassicCanPayload pack_0xff_and_tail(const uint8_t b) {
        const auto arr = std::array<uint8_t, 8>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, b};
        return hal::ClassicCanPayload::from_u8x8(arr);
    }

    static constexpr hal::ClassicCanPayload pack_query_param_data(const NodeId send_node_id, uint8_t reg_addr){
        const auto arr = std::array<uint8_t, 8>{
            static_cast<uint8_t>(send_node_id & 0xFF),
            static_cast<uint8_t>((send_node_id >> 8) & 0xFF),
            0x33,
            reg_addr,
            0x00, 0x00, 0x00, 0x00
        };
        return hal::ClassicCanPayload::from_u8x8(arr);
    }


    static constexpr hal::ClassicCanFrame pack_write_param(
        const hal::CanStdId motor_id, 
        const uint8_t reg_addr, 
        std::array<uint8_t, 4> bytes
    ) {
        const uint16_t id_u11 = motor_id.to_u11();
        const auto arr = std::array<uint8_t, 8>{
            static_cast<uint8_t>(id_u11 & 0xFF),
            static_cast<uint8_t>((id_u11 >> 8) & 0xFF),
            0x55,
            reg_addr,
            bytes[0], bytes[1], bytes[2], bytes[3]
        };
        return hal::ClassicCanFrame::from_parts(NMT_CAN_FRAME_ID, hal::ClassicCanPayload::from_u8x8(arr));
    }

    static constexpr hal::ClassicCanFrame pack_save_param(
        const hal::CanStdId motor_id
    ) {
        const uint16_t id_u11 = motor_id.to_u11();
        const auto arr = std::array<uint8_t, 8>{
            static_cast<uint8_t>(id_u11 & 0xFF),
            static_cast<uint8_t>((id_u11 >> 8) & 0xFF),
            0xaa,
            0x01,
            0x00, 0x00, 0x00, 0x00
        };
        return hal::ClassicCanFrame::from_parts(NMT_CAN_FRAME_ID, hal::ClassicCanPayload::from_u8x8(arr));
    }
};



}