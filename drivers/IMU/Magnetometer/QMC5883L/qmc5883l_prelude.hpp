#pragma once

#include "core/io/regs.hpp"
#include "core/utils/enum_array.hpp"
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
};

struct QMC5883L_Regs:public QMC5883L_Prelude{
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

    struct MagXReg:public Reg16i<>{
        static constexpr auto ADDRESS = RegAddr::MagX;
        int16_t :16;
    };

    struct MagYReg:public Reg16i<>{
        static constexpr auto ADDRESS = RegAddr::MagY;

        int16_t :16;
    };

    struct MagZReg:public Reg16i<>{
        static constexpr auto ADDRESS = RegAddr::MagZ;

        int16_t :16;
    };

    struct StatusReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Status;

        uint8_t ready:1;
        uint8_t ovl:1;
        uint8_t lock:1;
        uint8_t __resv__:5;
    };

    struct TemperatureReg:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::Tempature;

        uint16_t data;
    };

    struct ConfigAReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ConfigA;
        uint8_t measureMode:2;
        Odr odr:2;
        FullScale fs:2;
        OverSampleRatio ovs_ratio:2;
        
    };

    struct ConfigBReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ConfigB;

        uint8_t intEn:1;
        uint8_t __resv__:5;
        uint8_t rol:1;
        uint8_t srst:1;
        
    };

    struct ResetPeriodReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ResetPeriod;

        uint8_t data;
    };

    struct ChipIDReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ChipID;

        uint8_t data;
    };


    MagXReg mag_x_reg;
    MagYReg mag_y_reg;
    MagZReg mag_z_reg;
    StatusReg status_reg;
    TemperatureReg temperature_reg;
    ConfigAReg config_a_reg;
    ConfigBReg config_b_reg;
    ResetPeriodReg reset_period_reg;
    ChipIDReg chip_id_reg;

};



}