#pragma once

#include "as5047_prelude.hpp"

namespace ymd::drivers{

class AS5047:
    public MagEncoderIntf,
    public AS5047_Prelude{
public:


    template<typename T = void>
    using IResult = Result<T, Error>;

public:
    explicit AS5047(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit AS5047(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit AS5047(Some<hal::SpiBase *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv{spi, rank}){;}

    [[nodiscard]] IResult<> init() ;

    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<Angular<uq32>> read_lap_angle() {
        return Ok(Angular<uq32>::from_turns(lap_turns_));
    }
private:
    using Regs = AS5047_Regs;

    hal::SpiDrv spi_drv_;
    Regs regs_ = {};

    uq32 lap_turns_ = 0;

    uint16_t get_position_data();

    [[nodiscard]] IResult<> write_reg(const RegAddr addr, const uint8_t data);
    [[nodiscard]] IResult<> read_reg(const RegAddr addr, uint8_t & data);

};

};