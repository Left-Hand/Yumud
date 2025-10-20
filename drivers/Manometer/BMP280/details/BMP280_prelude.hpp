#pragma once

#include "core/io/regs.hpp"
#include "core/math/real.hpp"
#include "core/utils/result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct BMP280_Prelude{
    enum class Mode:uint8_t{
        Sleep, Single, Cont = 0x03
    };

    enum class TempratureSampleMode:uint8_t{
        SKIP, Bit16, Bit17, Bit18, Bit19, Bit20 = 0x07
    };

    enum class PressureSampleMode:uint8_t{
        SKIP, Bit16, Bit17, Bit18, Bit19, Bit20 = 0x07
    };

    enum class DataRate:uint8_t{
        HZ200, HZ16, HZ8, HZ4, HZ2, HZ1, HZ0_5, HZ0_25
    };

    enum class FilterCoefficient{
        OFF, FC2, FC4, FC8, FC16
    };

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0xec >> 1);
    static constexpr uint8_t VALID_CHIPID = 0x58;
    static constexpr uint8_t RESET_KEY = 0xB6;

    using RegAddr = uint8_t;

    enum class Error_Kind{
        WrongChipId,
        NoPressure
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
};

struct BMP280_Regs : public BMP280_Prelude {

    struct ChipIDReg:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xD0;
        uint8_t :8;
    };

    struct ResetReg:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xE0;

        using Reg8<>::operator=;
        uint8_t :8;
    };

    struct StatusReg:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xF3;
        uint8_t im:1;
        uint8_t __resv1__:2;
        uint8_t busy:1;
        uint8_t __resv2__:4;
    };

    struct CtrlReg:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xF4;
        uint8_t mode:2;
        uint8_t osrs_p:3;
        uint8_t osrs_t:3;
    };

    struct ConfigReg:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xF5;
        uint8_t spi3_en:1;
        uint8_t __resv__:1;
        uint8_t filter:3;
        uint8_t t_sb:3;
    };

    struct PressureReg:public Reg16<>{
        static constexpr RegAddr ADDRESS=0xF6;
        uint16_t:16;
    };

    struct PressureXReg:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xF9;
        uint8_t:8;
    };

    struct TemperatureReg:public Reg16<>{
        static constexpr RegAddr ADDRESS=0xFA;
        uint16_t:16;
    };

    struct TemperatureXReg:public Reg8<>{
        static constexpr RegAddr ADDRESS=0xFC;
        uint8_t:8;
    };


    uint16_t digT1;
    int16_t digT2;
    int16_t digT3;
    uint16_t digP1;
    int16_t digP2;
    int16_t digP3;
    int16_t digP4;
    int16_t digP5;
    int16_t digP6;
    int16_t digP7;
    int16_t digP8;
    int16_t digP9;
    ChipIDReg chip_id_reg;
    ResetReg reset_reg;
    StatusReg status_reg;
    CtrlReg ctrl_reg;
    ConfigReg config_reg;
    PressureReg pressure_reg;
    PressureXReg pressure_x_reg;
    TemperatureReg temperature_reg;
    TemperatureXReg temperature_x_reg;

    static constexpr uint8_t RESET_REG_ADDR = 0xE0;
    static constexpr uint8_t DIGT1_REG_ADDR = 0x88;
    static constexpr uint8_t CTRL_REG_ADDR = 0xF4;
};

}