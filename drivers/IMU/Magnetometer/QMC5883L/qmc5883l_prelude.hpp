#pragma once

#include "core/io/regs.hpp"
#include "core/utils/enum/enum_array.hpp"
#include "drivers/IMU/IMU.hpp"
#include <tuple>

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct QMC5883L_Prelude{

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x1a >> 1);

    using Error = ImuError;
    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Mode:uint8_t{
        Single,Continuous
    };

    enum class Odr:uint8_t{
        _10, _50, _100, _200
    };

    enum class OverSampleRatio:uint8_t{
        _512, 
        _256, 
        _128, 
        _64
    };

    enum class FullScale:uint8_t{
        _2G, 
        _8G
    };

    enum class RegAddr:uint8_t{
        MagX = 0x00,
        MagY = 0x02,
        MagZ = 0x04,
        Status = 0x06,
        Tempature = 0x07,
        ConfigA = 0x09,
        ConfigB = 0x0A,
        ResetPeriod = 0x0B,
        ChipID = 0x0D
    };
};

struct QMC5883L_Regs:public QMC5883L_Prelude{


    struct NestedXyz{
        int16_t x;
        int16_t y;
        int16_t z;
    };

    struct R8_Status:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Status;

        uint8_t ready:1;
        uint8_t ovl:1;
        uint8_t lock:1;
        uint8_t __resv__:5;
    };

    struct R16_Temperature:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Tempature;

        uint16_t bits;
    };

    struct R8_ConfigA:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::ConfigA;
        uint8_t measure_mode:2;
        Odr odr:2;
        FullScale fs:2;
        OverSampleRatio ovs_ratio:2;
    };

    struct R8_ConfigB:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::ConfigB;

        uint8_t int_en:1;
        uint8_t __resv__:5;
        uint8_t rol:1;
        uint8_t soft_reset:1;
        
    };

    struct R8_ResetPeriod:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::ResetPeriod;

        uint8_t bits;
    };

    struct R8_ChipId:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::ChipID;

        uint8_t bits;
    };

    NestedXyz xyz;
    R8_Status status_reg;
    R16_Temperature temperature_reg;
    R8_ConfigA config_a_reg;
    R8_ConfigB config_b_reg;
    R8_ResetPeriod reset_period_reg;
    R8_ChipId chip_id_reg;

};



}