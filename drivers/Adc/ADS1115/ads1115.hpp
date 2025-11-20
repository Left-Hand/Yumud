#pragma once

#include "ads1115_prelude.hpp"

namespace ymd::drivers{


class ADS111X final:
    public ADS111X_Regs{
public:
    explicit ADS111X(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit ADS111X(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit ADS111X(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};

    [[nodiscard]] IResult<> start_conv();

    [[nodiscard]] IResult<> set_threshold(int16_t low, int16_t high);

    [[nodiscard]] IResult<> enable_cont_mode(const Enable en);

    [[nodiscard]] IResult<> set_pga(const PGA pga);

    [[nodiscard]] IResult<> set_mux(const MUX mux);

    [[nodiscard]] IResult<> set_data_rate(const DataRate data_rate);

    [[nodiscard]] IResult<bool> is_busy();

    [[nodiscard]] Option<real_t> get_voltage();

    [[nodiscard]] IResult<> validate();

private:
    hal::I2cDrv i2c_drv_;

    [[nodiscard]] IResult<> read_reg(const RegAddr addr, uint16_t & data);

    [[nodiscard]] IResult<> write_reg(const RegAddr addr, const uint16_t data); 

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = write_reg(T::ADDRESS, reg.to_bits());
        if(res.is_ok()) reg.apply();
        return res;
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(T::ADDRESS, reg.as_bits_mut());
    }


};

using ADS1113 = ADS111X;
using ADS1114 = ADS111X;
using ADS1115 = ADS111X;


};