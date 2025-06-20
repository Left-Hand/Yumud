#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/errno.hpp"

#include "concept/pwm_channel.hpp"

#include "hal/gpio/vport.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{
struct PCA9685_Collections{
public:

    enum class RegAddress:uint8_t{
        Mode1,
        Mode2,
        LED0_ON_L = 0x06,
        LED0_ON_H,
        LED0_OFF_L,
        LED0_OFF_H,
        SubAddr = 0x02,
        Prescale = 0xfe
    };

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1000000);
    scexpr auto CHANNELS_COUNT = 16;

    enum class Error_Kind{
        IndexOutOfRange
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    struct Config{
        uint freq; 
        real_t trim = real_t(1);
    };
};

struct PCA9685_Regs :public PCA9685_Collections{

    struct Mode1Reg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Mode1;
        
        uint8_t allcall:1;
        uint8_t sub:3;
        uint8_t sleep:1;
        uint8_t auto_inc:1;
        uint8_t extclk:1;
        uint8_t restart:1;
    }DEF_R8(mode1_reg)

    struct Mode2Reg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Mode2;

        uint8_t outne:2;
        uint8_t outdrv:1;
        uint8_t och:1;
        uint8_t invrt:1;
        uint8_t __resv__:3;
    }DEF_R8(mode2_reg)

    struct LedOnOffReg:public Reg16<>{
        // static constexpr auto ADDRESS = RegAddress::Led;

        uint16_t cvr:12 = 0;
        uint16_t full:1 = 0;
        const uint16_t __resv__:3 = 0;
    };

    struct LedRegs{
        LedOnOffReg  on;
        LedOnOffReg off;
    };

    struct PrescaleReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Prescale;

        uint8_t prescale:8;
    }DEF_R8(prescale_reg)


    std::array<uint8_t,3> sub_addr_regs = {};
    uint8_t all_addr_reg = {};
    std::array<LedRegs, CHANNELS_COUNT> sub_channels = {};
    LedRegs all_channel = {};
};

}