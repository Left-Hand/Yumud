#pragma once

#include "mt6826s_prelude.hpp"

namespace ymd::drivers{


struct MT6826S final: public MT6826S_Prelude
{

    explicit MT6826S(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){}

    explicit MT6826S(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){}

    explicit MT6826S(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){}

    IResult<> init();
    IResult<Angular<uq32>> get_angle();
    IResult<EncoderFaultBitFields> get_fault();
    // [[nodiscard]] IResult<void> update();


private:
    hal::SpiDrv spi_drv_;

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        const auto address = T::ADDRESS;
        const uint8_t data = reg.to_bits();
        const auto tx = uint16_t(
            0x8000 | (std::bit_cast<uint8_t>(address) << 8) | data);
        if(const auto res = spi_drv_.write_single<uint16_t>(tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        reg.commit_changes();
        return Ok();
    }


    template<typename T>
    IResult<> read_packet(Packet & packet){
        return Ok();
    }
};

}