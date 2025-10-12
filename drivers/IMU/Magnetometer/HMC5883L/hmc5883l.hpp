#pragma once

#include "hmc5883l_prelude.hpp"

namespace ymd::drivers{


class HMC5883L final:
    public MagnetometerIntf,
    public HMC5883L_Regs{
public:
    explicit HMC5883L(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit HMC5883L(hal::I2cDrv && i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit HMC5883L(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> enable_high_speed(const Enable en);

    [[nodiscard]] IResult<> set_odr(const Odr rate);
    [[nodiscard]] IResult<> set_sample_number(const SampleNumber number);

    [[nodiscard]] IResult<> set_gain(const Gain gain);
    [[nodiscard]] IResult<> set_mode(const Mode mode);

    [[nodiscard]] IResult<Vec3<q24>> read_mag();

    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> update();


    [[nodiscard]] IResult<bool> is_data_ready();
private:

    hal::I2cDrv i2c_drv_;

    q16 lsb_ = 0;


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(reg.address), reg.as_val(), MSB);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(reg.address), reg.as_ref(), MSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const RegAddr addr, std::span<int16_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf, MSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    void set_lsb(const Gain gain){
        lsb_ = transfrom_gain_into_lsb(gain);
    }

    static constexpr q24 transfrom_gain_into_lsb(const Gain gain){
        switch(gain){
        case Gain::GL0_73:
            return q24(0.73);
        case Gain::GL0_92:
            return q24(0.92);
        case Gain::GL1_22:
            return q24(1.22);
        case Gain::GL1_52:
            return q24(1.52);
        case Gain::GL2_27:
            return q24(2.27);
        case Gain::GL2_56:
            return q24(2.56);
        case Gain::GL3_03:
            return q24(3.03);
        case Gain::GL4_35:
            return q24(4.35);
        default: __builtin_unreachable();
        }
    }

    static constexpr q16 transform_raw_to_gauss(const uint16_t data, const q24 lsb){
        return s16_to_uni(data & 0x8fff) * lsb;
    }
};

};