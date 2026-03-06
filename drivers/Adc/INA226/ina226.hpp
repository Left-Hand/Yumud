#pragma once

#include "ina226_prelude.hpp"

namespace ymd::drivers{

class INA226 final:public INA226_Prelude{
public:
    
    struct Config{
        const uint32_t average_times = 16;
        const ConversionTime bus_conv_time = ConversionTime::_140us;
        const ConversionTime shunt_conv_time = ConversionTime::_140us;
        const uint32_t sample_res_mohms;
        const uint32_t max_current_a;
    };


    explicit INA226(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit INA226(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit INA226(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};


    IResult<> init(const Config & cfg);

    IResult<> set_scale(const uint32_t mohms, const uint32_t max_current_a);

    IResult<> validate();

    IResult<> update();

    IResult<> set_average_times(const uint16_t times);

    IResult<BusVoltageCode> get_bus_voltage_code();

    IResult<ShuntVoltageCode> get_shunt_voltage_code();

    IResult<iq16> get_current();

    IResult<iq16> get_power();

    IResult<> set_average_times(const AverageTimes times);

    IResult<> set_bus_conversion_time(const ConversionTime time);

    IResult<> set_shunt_conversion_time(const ConversionTime time);

    IResult<> reset();

    IResult<> enable_shunt_voltage_measure(const Enable en);

    IResult<> enable_bus_voltage_measure(const Enable en);

    IResult<> enable_continuous_measure(const Enable en);

    IResult<> enable_alert_latch(const Enable en);
private:
    hal::I2cDrv i2c_drv_;
    INA226_Regs regs_ = {};
    
    iq16 current_lsb_ma_ = iq16(0.2);


    IResult<> write_reg(const RegAddr addr, const uint16_t data);

    IResult<> read_reg(const RegAddr addr, uint16_t & data);
    
    IResult<> read_reg(const RegAddr addr, int16_t & data);

    template<typename T>
    IResult<> read_reg(T & reg){
        return read_reg(T::REG_ADDR, reg.as_bits_mut());
    }
    
    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
};


}
