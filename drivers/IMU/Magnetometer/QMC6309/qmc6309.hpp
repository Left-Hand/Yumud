#pragma once

#include "qmc6309_prelude.hpp"

namespace ymd::drivers{


class QMC6309:
    public QMC6309_Prelude{
public:
    explicit QMC6309(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    explicit QMC6309(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    explicit QMC6309(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, i2c_addr)){;}


    QMC6309(const QMC6309 & other) = delete;
    QMC6309(QMC6309 && other) = delete;

    IResult<> init();

    IResult<> enable_cont_mode(const Enable en);
    
    IResult<> set_odr(const Odr rate);
    
    IResult<> set_fs(const FullScale fullscale);
    
    IResult<> set_over_sample_ratio(const OverSampleRatio ratio);

    IResult<> update();

    IResult<math::Vec3<iq16>> read_mag();
    
    IResult<> validate();

    IResult<> set_reset_period(const uint8_t resetPeriod);

    IResult<> reset();

    IResult<> enable_interrupt(const Enable en);

    IResult<bool> is_overflow();
private:
    QMC6309_Regs regs_;
    hal::I2cDrv i2c_drv_;

    // static constexpr EnumArray<FullScale, iq24> scaler_mapping_ = {
    //     2, 8
    // };

    // static constexpr EnumScaler<FullScale, iq24> scaler_ = {
    //     FullScale::_2G,
    //     scaler_mapping_
    // };
    iq16 scaler_ = 2;

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(T::REG_ADDR), reg.to_bits(), std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(T::REG_ADDR), reg.as_bits_mut(), std::endian::little);
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }


    IResult<> read_burst(
        const RegAddr reg_addr, 
        std::span<int16_t> pbuf
    ){
        if(const auto res = i2c_drv_.read_burst(uint8_t(reg_addr), pbuf, std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


    IResult<bool> is_busy();
};


}