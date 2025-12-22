#pragma once

#include "ina226_prelude.hpp"

namespace ymd::drivers{

class INA226 final:public INA226_Regs{
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


    [[nodiscard]] IResult<> init(const Config & cfg);

    [[nodiscard]] IResult<> set_scale(const uint32_t mohms, const uint32_t max_current_a);

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_average_times(const uint16_t times);

    [[nodiscard]] IResult<iq16> get_voltage();

    [[nodiscard]] IResult<int> get_shunt_voltage_uv();

    [[nodiscard]] IResult<iq16> get_shunt_voltage();

    [[nodiscard]] IResult<iq16> get_current();

    [[nodiscard]] IResult<iq16> get_power();

    [[nodiscard]] IResult<> set_average_times(const AverageTimes times);

    [[nodiscard]] IResult<> set_bus_conversion_time(const ConversionTime time);

    [[nodiscard]] IResult<> set_shunt_conversion_time(const ConversionTime time);

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> enable_shunt_voltage_measure(const Enable en);

    [[nodiscard]] IResult<> enable_bus_voltage_measure(const Enable en);

    [[nodiscard]] IResult<> enable_continuous_measure(const Enable en);

    [[nodiscard]] IResult<> enable_alert_latch(const Enable en);
private:
    hal::I2cDrv i2c_drv_;
    
    iq16 current_lsb_ma_ = iq16(0.2);


    [[nodiscard]] IResult<> write_reg(const RegAddr addr, const uint16_t data);

    [[nodiscard]] IResult<> read_reg(const RegAddr addr, uint16_t & data);
    
    [[nodiscard]] IResult<> read_reg(const RegAddr addr, int16_t & data);

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(T::ADDRESS, reg.as_bits_mut());
    }
    
    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
};


}
