#pragma once

#include "ina226_prelude.hpp"

namespace ymd::drivers{

class INA226 final:public INA226_Prelude{
public:
    
    struct [[nodiscard]] Config final{
        const AverageTimes average_times;
        const ConversionTime bus_conv_time;
        const ConversionTime shunt_conv_time;
        const uint32_t sample_res_mohms;
        const uint32_t max_current_ma;

        static constexpr Config from_default(){
            return Config{
                .average_times = AverageTimes::_16,
                .bus_conv_time = ConversionTime::_140us,
                .shunt_conv_time = ConversionTime::_140us,
                .sample_res_mohms = 5, // 0.005ohms
                .max_current_ma = 10 * 1000 // 10A
            };
        }
    };


    explicit INA226(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit INA226(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit INA226(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, i2c_addr)){};

    INA226(const INA226 &) = delete;
    INA226(INA226 &&) = delete;
    ~INA226() = default;



    IResult<> init(const Config & cfg);

    IResult<> validate();

    IResult<> soft_reset();

    IResult<> update();


    IResult<BusbarVoltageCode> get_busbar_voltage_code();

    IResult<ShuntVoltageCode> get_shunt_voltage_code();

    IResult<iq16> get_current();

    IResult<iq16> get_power();

    IResult<> set_average_times(const AverageTimes times);

    IResult<> set_busbar_conversion_time(const ConversionTime time);

    IResult<> set_shunt_conversion_time(const ConversionTime time);


    IResult<> enable_shunt_measure(const Enable en);

    IResult<> enable_busbar_measure(const Enable en);

    IResult<> enable_continuous_measure(const Enable en);

    IResult<> enable_alert_latch(const Enable en);
private:
    hal::I2cDrv i2c_drv_;

    using Regset = INA226_Regs;
    Regset regs_ = {};
    
    iq16 current_lsb_ma_ = iq16(0.2);


    IResult<> write_reg(const RegAddr reg_addr, const uint16_t reg_val);

    IResult<> read_reg(const RegAddr reg_addr, uint16_t & reg_val);
    
    IResult<> read_reg(const RegAddr reg_addr, int16_t & reg_val);

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

    IResult<> set_scale(const uint32_t mohms, const uint32_t max_current_amps);
};


}
