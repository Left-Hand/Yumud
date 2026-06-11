#pragma once

#include <tuple>

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"


// VEML7700 is a high accuracy ambient light digital 16-bit
// resolution sensor in a miniature transparent 6.8 mm x
// 2.35 mm x 3.0 mm package. It includes a high sensitive
// photo diode, a low noise amplifier, a 16-bit A/D converter
// and supports an easy to use I2C bus communication
// interface.
// The ambient light result is as digital value available

namespace ymd::drivers{

struct VEML7700_Prelude{

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1000100);

    enum class Error_Kind{

    };
    
    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class [[nodiscard]] RegAddr:uint8_t{
        ALS_CONF_0 = 0x00,
        ALS_WH = 0x01,
        ALS_WL,
        PowerSaving,
        ALS,
        WHITE,
        ALS_INT
    };

    enum class [[nodiscard]] IntegrationTime:uint8_t{
        _100ms = 0b0000,
        _200ms = 0b0001,
        _400ms = 0b0010,
        _800ms = 0b0011,
        _25ms = 0b1100,
        _50ms = 0b1000,
    };

    enum class [[nodiscard]] AlsGain:uint8_t{
        X1 = 0b00,
        X2 = 0b01,
        _1By8 = 0b10,
        _1By4 = 0b11,
    };

    enum class [[nodiscard]] PersistenceProtectNumber:uint8_t{
        _1 = 0b00,
        _2 = 0b01,
        _4 = 0b10,
        _8 = 0b11
    };

};

struct VEML7700_Regset:public VEML7700_Prelude{
    // CONFIGURATION REGISTER #0
    struct R16_Config:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0};

        // ALS shut down setting
        // 0 = ALS power on
        // 1 = ALS shut down
        uint16_t als_sd:1;

        // ALS interrupt enable setting
        // 0 = ALS INT disable
        // 1 = ALS INT enable
        uint16_t als_int_en:1;
        
        uint16_t __resv1__ :2;

    };
};


struct VEML7700:public VEML7700_Prelude{
    explicit VEML7700(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit VEML7700(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit VEML7700(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):
        i2c_drv_(i2c, i2c_addr){;}

    VEML7700(const VEML7700 &) = delete;
    VEML7700(VEML7700 &&) = delete;
    ~VEML7700() = default;
private:
    hal::I2cDrv i2c_drv_;

    using Regs = VEML7700_Regset;
    Regs regs_ = {};


    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(
            uint8_t(T::REG_ADDR), 
            reg.to_bits(), std::endian::little
        ); res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    

    IResult<> read_bulk(const RegAddr reg_addr, const std::span<uint16_t> pbuf);
};


}