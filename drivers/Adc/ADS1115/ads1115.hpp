#pragma once

#include "ads1115_prelude.hpp"


namespace ymd::drivers{


class ADS111X final:
    public ADS111X_Prelude{
public:
    explicit ADS111X(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit ADS111X(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit ADS111X(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, i2c_addr)){};

    IResult<> start_conv();

    IResult<> set_threshold(int16_t low, int16_t high);

    IResult<> enable_cont_mode(const Enable en);

    IResult<> set_pga(const PGA pga);

    IResult<> set_mux(const MUX mux);

    IResult<> set_datarate(const DataRate datarate);

    IResult<bool> is_busy();

    [[nodiscard]] Option<int16_t> get_conversion_result();

    IResult<> validate();

private:
    using Regset = ADS111X_Regs;
    Regset regs_;
    hal::I2cDrv i2c_drv_;

    IResult<> read_reg(const uint8_t reg_addr, uint16_t & reg_val);

    IResult<> write_reg(const uint8_t reg_addr, const uint16_t reg_val); 

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = write_reg(T::REG_ADDR, reg.to_bits());
        if(res.is_ok()) reg.apply();
        return res;
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        return read_reg(T::REG_ADDR, reg.as_bits_mut());
    }
};

using ADS1113 = ADS111X;
using ADS1114 = ADS111X;
using ADS1115 = ADS111X;


};