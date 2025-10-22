#pragma once

#include <tuple>

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"



namespace ymd::drivers{

struct TCS34725_Prelude{
    enum class Gain:uint8_t{
        _1x = 0b00, 
        _4x = 0b01, 
        _16x = 0b10, 
        _60x = 0b11 
    };

    enum class Error_Kind:uint8_t{
        WrongChipId
    };

    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
    

    enum class RegAddr:uint8_t{
        Enable = 0x00,
        Integration = 0x01,
        WaitTime = 0x03,
        LowThr = 0x04,
        HighThr = 0x06,
        IntPersistence = 0x0C,
        LongWait = 0x0D,
        Gain = 0x0F,
        DeviceId = 0x12,
        Status = 0x13,
        ClearData = 0x14,
        RedData = 0x16,
        GreenData = 0x18,
        BlueData = 0x1A
    };

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x52 >> 1);
};

struct TCS34725_Regset final:public TCS34725_Prelude{
    struct R8_Enable:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Enable;
        uint8_t powerOn : 1;
        uint8_t adc_en : 1;
        uint8_t __resv1__ :1;
        uint8_t wait_en : 1;
        uint8_t int_en : 1;
        uint8_t __resv2__ :3;
    }DEF_R8(enable_reg)

    struct R8_Integration:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Integration;
        uint8_t data;
    }DEF_R8(integration_reg)

    struct R8_IntPersistence:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::IntPersistence;
        using Reg8::operator=;
        uint8_t __resv__ :4;
        uint8_t apers   :4;
    }DEF_R8(int_persistence_reg)

    struct R8_LongWait:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::LongWait;
        uint8_t __resv1__ :1;
        uint8_t wait_long : 1;
        uint8_t __resv2__ :6;
    }DEF_R8(long_wait_reg)

    struct R8_Gain:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Gain;
        using Reg8::operator=;
        Gain gain        :2;
        uint8_t __resv2__   :6;
    }DEF_R8(gain_reg)

    struct R8_Status:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Status;
        uint8_t done_flag    :1;
        uint8_t __resv1__   :3;
        uint8_t interrupt_flag     :1;
        uint8_t __resv2__   :3;
    }DEF_R8(status_reg)


    struct R8_WaitTime:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::WaitTime;
        uint8_t data;
    }DEF_R8(wait_time_reg)

    struct R16_LowThr:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::LowThr;
        uint16_t data;
    }DEF_R16(low_thr_reg)

    struct R16_HighThr:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::HighThr;
        uint16_t data;
    }DEF_R16(high_thr_reg)

    struct R8_DeviceId:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::DeviceId;

        // 0x44 = TCS34721 and TCS34725
        // 0x4D = TCS34723 and TCS34727

        static constexpr uint8_t KEY = 0x44;
        uint8_t id;
    }DEF_R8(device_id_reg)


};


};