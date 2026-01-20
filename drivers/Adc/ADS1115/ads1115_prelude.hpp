#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/Option.hpp"



#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{


struct ADS111X_Prelude{

    enum class Error_Kind:uint8_t{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)

    using RegAddr = uint8_t;

    // ADDR PIN CONNECTION SLAVE ADDRESS
    // GND 1001000
    // VDD 1001001
    // SDA 1001010
    // SCL 1001011
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1001000);

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Package:uint8_t{
        ADS1113,
        ADS1114,
        ADS1115
    };

    enum class DataRate:uint8_t{
        _8 = 0b000,
        _16, _32, _64, _128, _250, _475, _860
    };

    enum class MUX:uint8_t{
        P0N1 = 0, P0N3, P1N3, P2N3, P0NG, P1NG, P2NG, P3NG
    };

    enum class PGA:uint8_t{
        _6_144V = 0, _4_096V, _2_048V, _1_024V, _0_512V, _0_256V
    };

};

struct ADS111X_Regs:public ADS111X_Prelude{ 

    struct ConversionReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0b00; 
        int16_t bits;
    };

    struct ConfigReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0b01; 
        uint16_t comp_que:2;
        uint16_t comp_latch:1;
        uint16_t comp_pol:1;

        uint16_t comp_mode_is_window:1;
        uint16_t data_rate:3;

        uint16_t oneshot_en:1;
        uint16_t pga:3;

        uint16_t mux:3;
        uint16_t busy:1;
    };

    struct LowThreshReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0b10;
        int16_t bits;
    };

    struct HighThreshReg:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0b11; 
        int16_t bits;
    };

    ConversionReg conversion_reg = {};
    ConfigReg config_reg = {};
    LowThreshReg low_thresh_reg = {};
    HighThreshReg high_thresh_reg = {};
};

};