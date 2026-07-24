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
    public AK09911C_Prelude{
public:

    explicit AK09911C(const hal::I2cDrv & i2c_drv):
        transport_(i2c_drv){;}
    explicit AK09911C(hal::I2cDrv && i2c_drv):
        transport_(i2c_drv){;}
    explicit AK09911C(Some<hal::I2cBase *> i2c, hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        transport_(hal::I2cDrv(i2c, addr)){;}
    explicit AK09911C(const hal::SpiDrv & spi_drv):
        transport_(spi_drv){;}
    explicit AK09911C(hal::SpiDrv && spi_drv):
        transport_(std::move(spi_drv)){;}
    explicit AK09911C(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        transport_(hal::SpiDrv(spi, rank)){;}

    IResult<> init();
    IResult<> update();
    IResult<> validate();
    IResult<bool> is_busy();
    IResult<bool> is_stable();
    IResult<> set_mode(const Mode mode);
    IResult<> disable_i2c();
    IResult<math::Vec3<iq24>> read_mag();

    IResult<bool> is_data_ready();
    IResult<bool> is_data_overrun();
    IResult<> enable_hs_i2c(const Enable en);
    IResult<> reset();
    IResult<> set_odr(const Odr odr);

private:
    using Regs = AK09911C_Regset;
    AsahiKaseiImu_Transport transport_;
    Regs regs_ = {};
    Option<math::Vec3<iq24>> scale_ = None; 
    
    IResult<> selftest();
    IResult<> blocking_update();
    IResult<> update_adj();

    IResult<> write_reg(const uint8_t reg_addr, const uint8_t reg_val){
        return transport_.write_reg(reg_addr, reg_val);
    }

    IResult<> read_reg(const uint8_t reg_addr, uint8_t & reg_val){
        return transport_.read_reg(reg_addr, reg_val);
    }

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(reg.REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    IResult<> read_reg(auto & reg){
        return transport_.read_reg(reg.REG_ADDR, reg.as_bits_mut());
    }

    
    IResult<> read_bulk(const RegAddr addr, std::span<int16_t> pbuf){
        return transport_.read_bulk(addr, pbuf);
    }

    IResult<math::Vec3<int8_t>> get_coeff();


};

};