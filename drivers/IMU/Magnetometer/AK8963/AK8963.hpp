#pragma once

#include "ak8963_prelude.hpp"

namespace ymd::drivers{


class AK8963:
    public MagnetometerIntf,
    public AK8963_Prelude{
public:
    explicit AK8963(const hal::I2cDrv & i2c_drv):
        phy_(i2c_drv){;}
    explicit AK8963(hal::I2cDrv && i2c_drv):
        phy_(i2c_drv){;}
    explicit AK8963(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv(i2c, addr)){;}
    explicit AK8963(const hal::SpiDrv & spi_drv):
        phy_(spi_drv){;}
    explicit AK8963(hal::SpiDrv && spi_drv):
        phy_(std::move(spi_drv)){;}
    explicit AK8963(Some<hal::SpiBase *> spi, const hal::SpiSlaveRank rank):
        phy_(hal::SpiDrv(spi, rank)){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<bool> is_busy();
    [[nodiscard]] IResult<bool> is_stable();
    [[nodiscard]] IResult<> disable_i2c();
    [[nodiscard]] IResult<Vec3<iq24>> read_mag();
    [[nodiscard]] IResult<> set_data_width(const uint8_t bits);
    [[nodiscard]] IResult<> set_mode(const Mode mode);
private:
    AsahiKaseiImu_Phy phy_;
    AK8963_Regs regs_ = {};

    bool data_valid_ = false;
    bool data_is_16_bits_ = false;

    Vec3<iq24> adj_scale_ = Vec3<iq24>::ZERO;


    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data);

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint8_t & data);


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = write_reg(T::ADDRESS, reg.to_bits());
        if(res.is_err()) return res;
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(T::ADDRESS, reg.as_bits_mut());
    }

    [[nodiscard]] IResult<> read_burst(const uint8_t reg_addr, const std::span<int16_t> pbuf);



    [[nodiscard]] IResult<Vec3<uint8_t>> get_coeff();


};
};