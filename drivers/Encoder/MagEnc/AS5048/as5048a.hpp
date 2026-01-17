#pragma once

//这个驱动还未完成编写
//这个驱动还未测试
//AS5048是一款18年的14位磁编码器 已经不推荐用于新设计

#include "as5048_prelude.hpp"

namespace ymd::drivers::as5048{

class AS5048A{
public:


    template<typename T = void>
    using IResult = Result<T, Error>;

public:
    explicit AS5048A(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit AS5048A(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit AS5048A(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv{spi, rank}){;}

    [[nodiscard]] IResult<> init() ;

    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<Angular<uq32>> read_lap_angle() {
        return Ok(Angular<uq32>::from_turns(lap_turns_));
    }
private:
    using Regs = AS5048A_Regs;

    hal::SpiDrv spi_drv_;
    Regs regs_ = {};

    uq32 lap_turns_ = 0;

    uint16_t get_position_data();

    [[nodiscard]] IResult<> write_reg(const uint16_t addr, const uint8_t data);
    [[nodiscard]] IResult<> read_reg(const uint16_t addr, uint8_t & data);

};

};