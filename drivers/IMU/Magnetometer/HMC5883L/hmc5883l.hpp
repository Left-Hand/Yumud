#pragma once

#include "hmc5883l_prelude.hpp"

namespace ymd::drivers{


class HMC5883L final:
    public HMC5883L_Prelude{
public:
    explicit HMC5883L(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit HMC5883L(hal::I2cDrv && i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit HMC5883L(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, i2c_addr)){;}

    IResult<> init();
    IResult<> enable_high_speed(const Enable en);

    IResult<> set_odr(const Odr rate);
    IResult<> set_sample_number(const SampleNumber number);

    IResult<> set_gain(const Gain gain);
    IResult<> set_mode(const Mode mode);

    IResult<math::Vec3<iq24>> read_mag();

    IResult<> validate();
    IResult<> update();


    IResult<bool> is_data_ready();
private:

    hal::I2cDrv i2c_drv_;
    HMC5883L_Regset regs_ = {};
    iq16 lsb_ = 0;


    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(T::REG_ADDR), reg.to_bits(), std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(T::REG_ADDR), reg.as_bits_mut(), std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_bulk(const RegAddr reg_addr, std::span<int16_t> pbuf){
        if(const auto res = i2c_drv_.read_bulk(uint8_t(reg_addr), pbuf, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_bulk(const RegAddr reg_addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_bulk(uint8_t(reg_addr), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    void set_lsb(const Gain gain);
};

};