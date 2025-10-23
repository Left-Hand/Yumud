#pragma once

//这款磁传感器不建议使用 原因如下：
// 1.I2c通信频率低
// 2.经常发生读写故障 需要多次重试才能正常通信
// 3.数据抖动大 直接转为姿态能有10度左右的抖动
// 4.很容易受环境磁场干扰

//注意
//市售的AK09911模块没有给RSTN接上拉电阻 导致模块默认处于复位状态

#include "ak09911c_prelude.hpp"

namespace ymd::drivers{

class AK09911C final:
    public MagnetometerIntf, 
    public AK09911C_Prelude{
public:

    explicit AK09911C(const hal::I2cDrv & i2c_drv):
        phy_(i2c_drv){;}
    explicit AK09911C(hal::I2cDrv && i2c_drv):
        phy_(i2c_drv){;}
    explicit AK09911C(Some<hal::I2c *> i2c, hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv(i2c, addr)){;}
    explicit AK09911C(const hal::SpiDrv & spi_drv):
        phy_(spi_drv){;}
    explicit AK09911C(hal::SpiDrv && spi_drv):
        phy_(std::move(spi_drv)){;}
    explicit AK09911C(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        phy_(hal::SpiDrv(spi, rank)){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<bool> is_busy();
    [[nodiscard]] IResult<bool> is_stable();
    [[nodiscard]] IResult<> set_mode(const Mode mode);
    [[nodiscard]] IResult<> disable_i2c();
    [[nodiscard]] IResult<Vec3<q24>> read_mag();

    [[nodiscard]] IResult<bool> is_data_ready();
    [[nodiscard]] IResult<bool> is_data_overrun();
    [[nodiscard]] IResult<> enable_hs_i2c(const Enable en);
    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<> set_odr(const Odr odr);

private:
    
    AsahiKaseiSensor_Phy phy_;
    AK09911C_Regset regs_ = {};
    Option<Vec3<q24>> scale_ = None; 
    
    [[nodiscard]] IResult<> selftest();
    [[nodiscard]] IResult<> blocking_update();
    [[nodiscard]] IResult<> update_adj();

    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data){
        return phy_.write_reg(addr, data);
    }

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint8_t & data){
        return phy_.read_reg(addr, data);
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(reg.address, reg.as_val());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    [[nodiscard]] IResult<> read_reg(auto & reg){
        return phy_.read_reg(reg.address, reg.as_ref());
    }

    
    [[nodiscard]] IResult<> read_burst(const RegAddress addr, std::span<int16_t> pbuf){
        return phy_.read_burst(addr, pbuf);
    }

    [[nodiscard]] IResult<Vec3<int8_t>> get_coeff();

    [[nodiscard]] static constexpr Vec3<q24> 
    transform_coeff_into_scale(const Vec3<int8_t> coeff){
        return Vec3<q24>(coeff) / 128 + Vec3<q24>(1, 1, 1); 
    }
    
};

};