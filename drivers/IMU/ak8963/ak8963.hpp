#pragma once

#include "ak8963_prelude.hpp"

namespace ymd::drivers{


class AK8963:
    public AK8963_Prelude{
public:
    explicit AK8963(const hal::I2cDrv & i2c_drv):
        transport_(i2c_drv){;}
    explicit AK8963(hal::I2cDrv && i2c_drv):
        transport_(i2c_drv){;}
    explicit AK8963(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        transport_(hal::I2cDrv(i2c, i2c_addr)){;}
    explicit AK8963(const hal::SpiDrv & spi_drv):
        transport_(spi_drv){;}
    explicit AK8963(hal::SpiDrv && spi_drv):
        transport_(std::move(spi_drv)){;}
    explicit AK8963(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        transport_(hal::SpiDrv(spi, rank)){;}

    IResult<> init();
    IResult<> update();
    IResult<> validate();
    IResult<> reset();
    IResult<bool> is_busy();
    IResult<bool> is_stable();
    IResult<> disable_i2c();
    IResult<math::Vec3<iq24>> read_mag();
    IResult<> set_data_width(const uint8_t bits);
    IResult<> set_mode(const Mode mode);
private:
    AsahiKaseiImu_Transport transport_;
    AK8963_Regs regs_ = {};

    bool data_valid_ = false;
    bool data_is_16_bits_ = false;

    math::Vec3<iq24> adj_scale_ = math::Vec3<iq24>::ZERO;


    IResult<> write_reg(const uint8_t addr, const uint8_t data);

    IResult<> read_reg(const uint8_t addr, uint8_t & data);


    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = write_reg(T::REG_ADDR, reg.to_bits());
        if(res.is_err()) return res;
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    IResult<> read_reg(T & reg){
        return read_reg(T::REG_ADDR, reg.as_bits_mut());
    }

    IResult<> read_bulk(const uint8_t reg_addr, const std::span<int16_t> pbuf);



    IResult<math::Vec3<uint8_t>> get_coeff();


};
};