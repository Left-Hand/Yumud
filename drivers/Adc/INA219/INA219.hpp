#pragma once

#include "core/io/regs.hpp"

#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "concept/analog_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

class INA219 {
public:


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

protected:
    hal::I2cDrv i2c_drv_;
    
    real_t current_lsb_ma = real_t(0.2);
    scexpr real_t voltage_lsb_mv = real_t(1.25);

    using RegAddress = uint8_t;

    struct R16_Config:public Reg16<>{
        scexpr RegAddress address = 0x00;

        uint16_t mode:3;
        uint16_t sdac:4;
        uint16_t bdac:4;
        uint16_t pg:2;
        uint16_t brng:1;
        uint16_t __resv__:1;
        uint16_t rst:1;
    }DEF_R16(config_reg)

    struct R16_ShuntVolt:public Reg16<>{
        scexpr RegAddress address = 0x01;
        uint16_t :16;
    }DEF_R16(shunt_volt_reg)

    struct R16_BusVolt:public Reg16<>{
        scexpr RegAddress address = 0x02;
        uint16_t :16;
    }DEF_R16(bus_volt_reg)

    struct R16_Power:public Reg16i<>{
        scexpr RegAddress address = 0x03;
        int16_t :16;
    }DEF_R16(power_reg)
    struct R16_Current:public Reg16i<>{
        scexpr RegAddress address = 0x04;
        int16_t :16;
    }DEF_R16(current_reg)
    
    struct R16_Calibration:public Reg16i<>{
        scexpr RegAddress address = 0x05;
        int16_t :16;
    }DEF_R16(calibration_reg)
    

    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const uint16_t data);

    [[nodiscard]] IResult<> read_reg(const RegAddress addr, uint16_t & data);
    
    [[nodiscard]] IResult<> read_reg(const RegAddress addr, int16_t & data);

    [[nodiscard]] IResult<> read_burst(const RegAddress addr, std::span<uint16_t> pdata);

public:
    
scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x80);

    #define CHANNEL_CONTENT\
        INA219Channel{*this, INA219Channel::Index::SHUNT_VOLT},\
        INA219Channel{*this, INA219Channel::Index::BUS_VOLT},\
        INA219Channel{*this, INA219Channel::Index::CURRENT},\
        INA219Channel{*this, INA219Channel::Index::POWER}\

    INA219(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    INA219(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    INA219(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};

    #undef CHANNEL_CONTENT


    IResult<> update();

    IResult<> validate();
};
}
