#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "primitive/analog_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct INA226_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x80 >> 1);
    // ASCII 的 TI。
    static constexpr uint16_t VALID_MANU_ID = 0x5449;

    // INA226
    static constexpr uint16_t VALID_CHIP_ID = 0x2260;

    enum class Error_Kind{
        ChipIdVerifyFailed,
        ManuIdVerifyFailed,
        ResTooBig
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class AverageTimes:uint16_t{
        _1 = 0,
        _4 = 1,
        _16 = 2,
        _64 = 3,
        _128 = 4,
        _256 = 5,
        _512 = 6,
        _1024 = 7
    };

    enum class ConversionTime:uint16_t{
        _140us = 0, _204us, _332us, _588us, _1_1ms, _2_116_ms, _4_156ms, _8_244ms
    };

    using RegAddr = uint8_t;

    static constexpr real_t VOLTAGE_LSB_MV = real_t(1.25);
};

struct INA226_Regs:public INA226_Prelude{
    struct ConfigReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x00;

        uint16_t shunt_voltage_enable :1;
        uint16_t bus_voltage_enable :1;
        uint16_t continuos :1;
        ConversionTime shunt_voltage_conversion_time:3;
        ConversionTime bus_voltage_conversion_time:3;
        AverageTimes average_times:3;
        uint16_t __resv__:3;
        uint16_t rst:1;
    }DEF_R16(config_reg)

    struct ShuntVoltReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x01;
        uint16_t bits;
    }DEF_R16(shunt_volt_reg)

    struct BusVoltReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x02;
        uint16_t bits;
    }DEF_R16(bus_volt_reg)

    struct PowerReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x03;
        int16_t bits;
    }DEF_R16(power_reg)

    struct CurrentReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x04;
        int16_t bits;
    }DEF_R16(current_reg)
    
    struct CalibrationReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x05;
        int16_t bits;
    }DEF_R16(calibration_reg)
    
    struct MaskReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x06;

        uint16_t alert_latch_enable:1;
        uint16_t alert_polarity:1;
        uint16_t math_overflow:1;
        uint16_t conv_ready_flag:1;
        uint16_t alert_flag:1;
        uint16_t __resv__:5;
        uint16_t conv_ready:1;
        uint16_t power_overlimit:1;
        uint16_t bus_under_voltage:1;
        uint16_t bus_over_voltage:1;
        uint16_t shunt_under_voltage:1;
        uint16_t shunt_over_voltage:1;
    }DEF_R16(mask_reg)

    struct AlertLimitReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x07;
        uint16_t bits;
    }DEF_R16(alert_limit_reg)

    struct ManufactureReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0xfe;
        uint16_t bits;
    }DEF_R16(manufacture_reg)

    struct ChipIdReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0xff;
        uint16_t bits;
    }DEF_R16(chip_id_reg)
};



}
