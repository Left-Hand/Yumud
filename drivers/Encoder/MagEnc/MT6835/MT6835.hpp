#pragma once

#include "mt6835_prelude.hpp"

namespace ymd::drivers{

class MT6835 final:
    public MagEncoderIntf,
    public MT6835_Regs{
public:
    explicit MT6835(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit MT6835(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit MT6835(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv{spi, rank}){;}

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<Angle<q31>> read_lap_angle(){
        return last_packet_.parse();
    }
private:
    hal::SpiDrv spi_drv_;
    Packet last_packet_;

    [[nodiscard]] IResult<> write_reg(const RegAddr reg_addr, const uint8_t reg_val);
    [[nodiscard]] IResult<> read_reg(const RegAddr reg_addr, uint8_t & reg_val);
};

};