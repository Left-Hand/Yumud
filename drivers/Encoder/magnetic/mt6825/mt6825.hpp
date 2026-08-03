#pragma once

//18位磁编码器
//已完成 已测试

//锐评：19年出品的依托答辩。从没见过回程差0.003圈 再低的spi速率校验位也频繁出错的编码器


#include "mt6825_prelude.hpp"

namespace ymd::drivers{


struct MT6825 final: public MT6825_Prelude{
    explicit MT6825(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){}

    explicit MT6825(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){}

    explicit MT6825(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){}


    IResult<Angular<uq32>> get_angle();
private:
    hal::SpiDrv spi_drv_;

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.as_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.commit_changes();
        return Ok();
    }

    template<typename T>
    IResult<> write_reg(const uint8_t reg_addr, const uint8_t reg_val){
        const auto tx = uint16_t(
            0x8000 | (reg_addr << 8) | reg_val);
        if(const auto res = spi_drv_.write_single<uint16_t>(tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        return Ok();
    }

    template<typename T>
    IResult<> read_reg(const T & reg){
        return read_reg(T::REG_ADDR, reg.as_bits_mut());
    }

    IResult<> read_reg(const uint8_t reg_addr, uint8_t & reg_val){
        const uint16_t tx = uint16_t(
            0x8000 | (reg_addr << 8) | reg_val);
        uint16_t rx;
        if(const auto res = spi_drv_.transceive_single<uint16_t>(rx, tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        reg_val = static_cast<uint8_t>(rx);
        return Ok();
    }


    IResult<Packet> read_packet();
};
}