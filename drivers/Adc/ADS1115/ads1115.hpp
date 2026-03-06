#pragma once

#include "ads1115_prelude.hpp"
#include "core/math/real.hpp"
#include "core/utils/Option.hpp"

namespace ymd::drivers{


class ADS111X final:
    public ADS111X_Regs{
public:
    explicit ADS111X(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit ADS111X(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit ADS111X(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};

    IResult<> start_conv();

    IResult<> set_threshold(int16_t low, int16_t high);

    IResult<> enable_cont_mode(const Enable en);

    IResult<> set_pga(const PGA pga);

    IResult<> set_mux(const MUX mux);

    IResult<> set_datarate(const DataRate datarate);

    IResult<bool> is_busy();

    [[nodiscard]] Option<iq16> get_voltage();

    IResult<> validate();

private:
    hal::I2cDrv i2c_drv_;

    IResult<> read_reg(const RegAddr addr, uint16_t & data);

    IResult<> write_reg(const RegAddr addr, const uint16_t data); 

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