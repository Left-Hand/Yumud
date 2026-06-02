#include "hightorque_packedcodes.hpp"
#include "hightorque_serialize.hpp"

namespace ymd::robots::hightorque{


namespace req_slots{

static constexpr uint16_t NIL_U16 = 0x8000;

struct [[nodiscard]] SetMode final{
    static constexpr size_t NUM_BYTES = 3;
    static constexpr RegAddr REG_ADDR = RegAddr::Mode;


    Mode mode;

    constexpr void fill_bytes(std::span<uint8_t, NUM_BYTES> bytes) const noexcept {
        constexpr auto filler = SlotFiller::from_write(REG_ADDR);
        filler.fill_bytes_from_elements(bytes, mode);
    }
};


struct [[nodiscard]] ControlQCurrent final{
    static constexpr size_t NUM_BYTES = 4;
    static constexpr RegAddr REG_ADDR = RegAddr{0x1c};
    
    CurrentCode q_current_code;

    constexpr void fill_bytes(std::span<uint8_t, NUM_BYTES> bytes) const noexcept {
        constexpr auto filler = SlotFiller::from_write(REG_ADDR);
        filler.fill_bytes_from_elements(bytes, q_current_code);
    }
};

struct [[nodiscard]] ControlPosition final{
    static constexpr size_t NUM_BYTES = 8;
    static constexpr RegAddr REG_ADDR = RegAddr{0x07};

    PositionCode position_code;
    
    TorqueCode torque_code;

    constexpr void fill_bytes(std::span<uint8_t, NUM_BYTES> bytes) const noexcept {
        constexpr auto filler = SlotFiller::from_write(REG_ADDR);
        filler.fill_bytes_from_elements(bytes, position_code, NIL_U16, torque_code);
    }
};

struct [[nodiscard]] ControlTorque final{
    static constexpr size_t NUM_BYTES = 4;
    static constexpr RegAddr REG_ADDR = RegAddr{0x13};

    
    
    TorqueCode torque_code;

    constexpr void fill_bytes(std::span<uint8_t, NUM_BYTES> bytes) const noexcept {
        constexpr auto filler = SlotFiller::from_write(REG_ADDR);
        filler.fill_bytes_from_elements(bytes, torque_code);
    }
};

// 电机位置-速度-最大力矩控制，int16型
struct [[nodiscard]] ControlPvt final{
    static constexpr size_t NUM_BYTES = 8;
    static constexpr RegAddr REG_ADDR = RegAddr{0x35};
    

    PositionCode position_code;

    SpeedCode speed_code;
    
    TorqueCode torque_code;

    constexpr void fill_bytes(std::span<uint8_t, NUM_BYTES> bytes) const noexcept {
        constexpr auto filler = SlotFiller::from_write(REG_ADDR);
        filler.fill_bytes_from_elements(bytes, position_code, speed_code, torque_code);
    }
};

// 梯形控制（电机固件 v4.6.0 开始支持）
struct [[nodiscard]] ControlTrapezoidal final{
    static constexpr size_t NUM_BYTES = 8;
    static constexpr RegAddr REG_ADDR = RegAddr{0x35};
    

    PositionCode position_code;
    
    SpeedCode speed_code;
    AccelerationCode accel_code;
    constexpr void fill_bytes(std::span<uint8_t, NUM_BYTES> bytes) const noexcept {
        constexpr auto filler = SlotFiller::from_write(REG_ADDR);
        filler.fill_bytes_from_elements(bytes, position_code, speed_code, accel_code);
    }
};

struct [[nodiscard]] RezeroPosition final{

};


};
}