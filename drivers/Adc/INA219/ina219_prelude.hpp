#pragma once

#include "core/io/regs.hpp"

#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/math/real.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct INA219_Prelude{
    using RegAddr = uint8_t;

    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u7(0x80 >> 1);

    enum class Error_Kind:uint8_t{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class AverageTimes:uint8_t{
        _1 = 0,
        _4 = 1,
        _16 = 2,
        _64 = 3,
        _128 = 4,
        _256 = 5,
        _512 = 6,
        _1024 = 7
    };

    enum class ConversionTime:uint8_t{
        _140us = 0, _204us, _332us, _588us, _1_1ms, _2_116_ms, _4_156ms, _8_244ms
    };

    static constexpr iq16 VOLTAGE_LSB_MV = iq16(1.25);

};


struct INA219_Regs:public INA219_Prelude{
    struct R16_Config:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x00;

        uint16_t mode:3;
        uint16_t sdac:4;
        uint16_t bdac:4;
        uint16_t pg:2;
        uint16_t brng:1;
        uint16_t __resv__:1;
        uint16_t rst:1;
    }DEF_R16(config_reg)

    struct R16_ShuntVolt:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x01;
        uint16_t bits;
    }DEF_R16(shunt_volt_reg)

    struct R16_BusVolt:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x02;
        uint16_t bits;
    }DEF_R16(bus_volt_reg)

    struct R16_Power:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x03;
        int16_t bits;
    }DEF_R16(power_reg)
    
    struct R16_Current:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x04;
        int16_t bits;
    }DEF_R16(current_reg)
    
    struct R16_Calibration:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x05;
        int16_t bits;
    }DEF_R16(calibration_reg)
};
}
