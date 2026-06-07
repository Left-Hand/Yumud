#pragma once

#include "evo_primitive.hpp"
#include "primitive/can/can_id.hpp"
#include "primitive/can/can_frame.hpp"

namespace ymd::robots::evo{

struct [[nodiscard]] FrameFactory final{

    const NodeId motor_id;

    constexpr hal::ClassicCanFrame enable(this auto && self) noexcept {
        return pack_0xff_command(self.motor_id, Command::Enable);
    }

    constexpr hal::ClassicCanFrame disable(this auto && self) noexcept {
        return pack_0xff_command(self.motor_id, Command::Disable);
    }

    constexpr hal::ClassicCanFrame set_zero(this auto && self) noexcept {
        return pack_0xff_command(self.motor_id, Command::SetZero);
    }

    constexpr hal::ClassicCanFrame get_param(this auto && self, const FlashParamId id) noexcept {
        const std::array<uint8_t, 8> u8x8 = {
            uint8_t(Command::StartFlash),
            uint8_t(id),
            0,0,0,0,
            uint8_t(Command::ReadFlash),
            uint8_t(Command::EndFlash),
        };

        return hal::ClassicCanFrame::from_parts(
            PARAM_BASE + self.motor_id, 
            hal::ClassicCanPayload::from_u8x8(u8x8)
        );
    }

    constexpr hal::ClassicCanFrame save_flash(this auto && self) noexcept {
        const std::array<uint8_t, 8> u8x8 = {
            uint8_t(Command::StartFlash),
            0,
            0,0,0,0,
            uint8_t(Command::SaveFlash),
            uint8_t(Command::EndFlash),
        };

        return hal::ClassicCanFrame::from_parts(
            PARAM_BASE + self.motor_id, 
            hal::ClassicCanPayload::from_u8x8(u8x8)
        );
    }


    template<typename T>
    requires (sizeof(T) == 4)
    constexpr hal::ClassicCanFrame write_reg(this auto && self, 
        const uint8_t reg_addr, 
        const T param
    ){
        const auto && param_bytes = std::bit_cast<std::array<uint8_t, 4>>(param);
        return pack_write_param(self.motor_id, reg_addr, param_bytes); 
    }


private:
    struct [[nodiscard]] IdBase final{
        uint16_t count;

        constexpr hal::CanStdId  operator +(const NodeId node_id) const noexcept {
            return hal::CanStdId::from_u11(count + node_id.count);
        }
    };

    static constexpr IdBase NO_BASE = {0x000};
    static constexpr IdBase PARAM_BASE = {0x600};

    static constexpr hal::CanStdId NMT_CAN_FRAME_ID = hal::CanStdId::from_u11(0x7FF);

    static constexpr hal::ClassicCanPayload pack_0xff_and_tail(const uint8_t b) noexcept {
        const auto arr = std::array<uint8_t, 8>{
            0xff, 0xff, 0xff, 0xff, 
            0xff, 0xff, 0xff, b
        };

        return hal::ClassicCanPayload::from_u8x8(arr);
    }


    static constexpr hal::ClassicCanFrame pack_0xff_command(
        const NodeId motor_id,
        const Command command
    ) noexcept {
        return hal::ClassicCanFrame::from_parts(
            NO_BASE + motor_id, 
            pack_0xff_and_tail(static_cast<uint8_t>(command))
        );
    }


    static constexpr hal::ClassicCanFrame pack_write_param(
        const NodeId motor_id, 
        const uint8_t reg_addr, 
        std::array<uint8_t, 4> bytes
    ) noexcept {
        const auto arr = std::array<uint8_t, 8>{
            uint8_t(Command::StartFlash),
            uint8_t(reg_addr),
            bytes[0], bytes[1], bytes[2], bytes[3],
            uint8_t(Command::ReadFlash),
            uint8_t(Command::EndFlash),
        };
        return hal::ClassicCanFrame::from_parts(
            PARAM_BASE + motor_id, 
            hal::ClassicCanPayload::from_u8x8(arr)
        );
    }


};

}