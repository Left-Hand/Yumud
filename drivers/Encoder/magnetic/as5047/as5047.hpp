#pragma once

#include "as5047_prelude.hpp"

namespace ymd::drivers{

class AS5047:public AS5047_Prelude{
public:
    template<typename T = void>
    using IResult = Result<T, Error>;

public:
    explicit AS5047(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit AS5047(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit AS5047(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv{spi, rank}){;}

    IResult<> init() ;

    IResult<> update();
    IResult<Angular<uq32>> get_angle();
private:
    using Regs = AS5047_Regs;

    Regs regs_ = {};
    hal::SpiDrv spi_drv_;


    uint16_t get_angle_bits();

    IResult<> write_reg(const Packet write_addr_pkt, const uint16_t reg_val);
    IResult<> read_reg(const Packet write_addr_pkt, uint16_t & reg_val);

};

};