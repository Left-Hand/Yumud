#pragma once

#include "qmc5883l_prelude.hpp"


namespace ymd::drivers{


class QMC5883L:
    public MagnetometerIntf,
    public QMC5883L_Regs{
public:
    QMC5883L(const QMC5883L & other) = delete;
    QMC5883L(QMC5883L && other) = delete;

    QMC5883L(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    QMC5883L(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    QMC5883L(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> enable_cont_mode(const Enable en);
    
    [[nodiscard]] IResult<> set_odr(const Odr rate);
    
    [[nodiscard]] IResult<> set_fs(const FullScale fullscale);
    
    [[nodiscard]] IResult<> set_over_sample_ratio(const OverSampleRatio ratio);

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<Vec3<q24>> read_mag() override;
    
    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> set_reset_period(const uint8_t resetPeriod);

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> enable_interrupt(const Enable en);

    [[nodiscard]] IResult<bool> is_overflow();
private:
    hal::I2cDrv i2c_drv_;

    static constexpr EnumArray<FullScale, q24> scaler_mapping_ = {
        2, 8
    };

    static constexpr EnumScaler<FullScale, q24> scaler_ = {
        FullScale::_2G,
        scaler_mapping_
    };


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(T::ADDRESS), reg.as_val(), LSB);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(T::ADDRESS), reg.as_ref(), LSB);
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }


    [[nodiscard]] IResult<> read_burst(
        const RegAddr addr, 
        std::span<int16_t> pbuf
    ){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf, LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


    IResult<bool> is_busy(){
        if(const auto res = read_reg(status_reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(status_reg.ready == false);
    }
};


}