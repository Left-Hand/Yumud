#pragma once

#include <optional>

#include "core/io/regs.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"


namespace ymd::drivers{

struct BMM150_Prelude{
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;


    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x68 >> 1);

    struct InterruptMask:public Reg8<>{
        uint8_t low_int_x:1;
        uint8_t low_int_y:1;
        uint8_t low_int_z:1;
        uint8_t high_int_x:1;
        uint8_t high_int_y:1;
        uint8_t high_int_z:1;
        uint8_t overflow:1;
        uint8_t data_overrun:1;
    };

    enum class OperationMode:uint8_t{
        Normal = 0b00,
        Forced = 0b01,
        __Reserved__ = 0b10,
        Sleep = 0b11
    };

    enum class DataRate:uint8_t{
        _10Hz = 0b000,
        _2Hz = 0b001,
        _6Hz = 0b010,
        _8Hz = 0b011,
        _15Hz = 0b100,
        _20Hz = 0b101,
        _25Hz = 0b110,
        _30kHz = 0b111,
        Default = _10Hz,
    };


    enum class RegAddr:uint8_t{
        ChipId = 0x40,
        X = 0x42,
        Y = 0x44,
        Y = 0x46,
        Rhall = 0x48,
        InterruptStatus = 0x4a,
        PowerControl = 0x4b,
        Control = 0x4c,
        InterruptEnable = 0x4d,
        AxisEnable = 0x4e,
        LowThresholdSetting = 0x4f,
        HighThresholdSetting = 0x50,
        XyRepetitions = 0x51,
        ZRepetitions = 0x52,
    };
};


struct BMM150_Regset:public BMM150_Prelude{

// 0x40
struct R8_ChipId{
    static constexpr auto ADDRESS = RegAddr::ChipId;
    static constexpr uint8_t KEY = 0b00110010;

    uint8_t chipid;
}DEF_R8(chipid_reg)

struct _R16_MagField{
    
    uint16_t selftest:1;
    uint16_t :2;
    uint16_t data:13;
};

//0x42
struct R16_X:public _R16_MagField{
    static constexpr auto ADDRESS = RegAddr::X;
}DEF_R16(x_reg)

//0x44
struct R16_Y:public _R16_MagField{
    static constexpr auto ADDRESS = RegAddr::Y;
}DEF_R16(y_reg)

//0x46
struct R16_Z:public _R16_MagField{
    static constexpr auto ADDRESS = RegAddr::Z;
}DEF_R16(z_reg)

//0x48
struct R16_Rhall{
    static constexpr auto ADDRESS = RegAddr::Rhall;
    uint16_t data_ready:1;
    uint16_t :1;
    uint16_t data:14;
}DEF_R16(rhall_reg)


//0x4a
struct R8_InterruptStatus:public InterruptMask{
    static constexpr auto ADDRESS = RegAddr::InterruptStatus;
}DEF_R16(interrupt_status_reg)

//0x4b
struct R8_PowerControl{
    static constexpr auto ADDRESS = RegAddr::PowerControl;
    uint8_t not_suspend:1;
    uint8_t soft_reset:1;
    uint8_t spi_3wire_en:1;
    uint8_t :4;
    uint8_t soft_reset2:1;
}DEF_R8(power_control_reg)

//0x4c
struct R8_Control{
    static constexpr auto ADDRESS = RegAddr::Control;
    uint8_t self_test:1;
    OperationMode opmode:2;
    DataRate datarate:3;

    // @Page27 see 4.4.2
    uint8_t advanced_self_test:2;
}DEF_R8(control_reg)

//0x4d
struct R8_InterruptEnable:public InterruptMask{
    static constexpr auto ADDRESS = RegAddr::InterruptEnable;
}DEF_R8(interrupt_enable_reg)

//0x4e
struct R8_AxisEnable{
    static constexpr auto ADDRESS = RegAddr::AxisEnable;
    uint8_t interrupt_polarity:1;
    uint8_t interrupt_latch:1;
    uint8_t dr_polarity:1;
    uint8_t channel_x:1;

    uint8_t channel_y:1;
    uint8_t channel_z:1;
    uint8_t interrupt_pin_en:1;
    uint8_t data_ready_pin_en:1;
}DEF_R8(axis_enable_reg)

//0x4f
struct R8_LowThresholdSetting{
    static constexpr auto ADDRESS = RegAddr::LowThresholdSetting;
    uint8_t _0:1;
    uint8_t _1:1;
    uint8_t _2:1;
    uint8_t _3:1;

    uint8_t _4:1;
    uint8_t _5:1;
    uint8_t _6:1;
    uint8_t _7:1;
}DEF_R8(low_threshold_setting)

//0x50
struct R8_HighThresholdSetting{
    static constexpr auto ADDRESS = RegAddr::HighThresholdSetting;
    uint8_t _0:1;
    uint8_t _1:1;
    uint8_t _2:1;
    uint8_t _3:1;

    uint8_t _4:1;
    uint8_t _5:1;
    uint8_t _6:1;
    uint8_t _7:1;
}DEF_R8(high_threshold_setting)



//0x51
struct R8_XyRepetitions{
    static constexpr auto ADDRESS = RegAddr::XyRepetitions;
    uint8_t data;

    constexpr void set_times(size_t times){

        if((times == 0) or (times > 511)) __builtin_trap();
        data = static_cast<uint8_t>((times - 1) >> 1);
    };
}DEF_R8(xy_repetitions_reg)

//0x52
struct R8_ZRepetitions{
    static constexpr auto ADDRESS = RegAddr::ZRepetitions;
    uint8_t data;

    constexpr void set_times(size_t times){
        if((times == 0) or (times > 256)) __builtin_trap();
        data = static_cast<uint8_t>((times - 1));
    };
}DEF_R8(z_repetitions_reg)
};
}