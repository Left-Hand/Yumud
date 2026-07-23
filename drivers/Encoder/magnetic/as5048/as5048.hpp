#pragma once

//这个驱动还未完成编写
//这个驱动还未测试
//AS5048是一款18年的14位磁编码器 已经不推荐用于新设计

#include "as5048_prelude.hpp"

namespace ymd::drivers::as5048{

class AS5048A:public AS5048_Prelude{
public:
    explicit AS5048A(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit AS5048A(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit AS5048A(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv{spi, rank}){;}

    IResult<> init() ;

    IResult<> update();
    IResult<Angular<uq32>> read_lap_angle() {
        return Ok(Angular<uq32>::from_turns(lap_turns_));
    }
private:
    using Regs = AS5048A_Regset;

    hal::SpiDrv spi_drv_;
    Regs regs_ = {};

    uq32 lap_turns_ = 0;

    uint16_t get_position_data();

    IResult<> write_reg(const uint16_t reg_addr, const uint8_t reg_val);
    IResult<> read_reg(const uint16_t reg_addr, uint8_t & reg_val);

};

class AS5048B:public AS5048_Prelude{
public:
    // 1 0 0 0 0 a1 a0
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b100'0000);

    explicit AS5048B(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit AS5048B(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    explicit AS5048B(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv{i2c, i2c_addr}){;}
private:
    using Regs = AS5048A_Regset;

    hal::I2cDrv i2c_drv_;
};


};