#pragma once 


#include "ina237_prelude.hpp"

// 这个驱动完成度较高

//   INA237 是一款超精密数字功率监控器，配备专为电流
// 检测应用而设计的 16 位 Δ-Σ ADC。该器件可跨共模
// 电压支持范围为 -0.3V 至 +85V 的电阻式分流器感测元
// 件测量 ±163.84mV 或 ±40.96mV 的满量程差分输入。

//   INA237 报告电流、总线电压、温度和功率，同时在后
// 台执行所需的计算。集成的温度传感器用于裸片温度测
// 量的精度为 ±1°C，并可用于监测系统环境温度。

//   INA237 采用低温漂和增益漂移设计，以使此器件可用
// 于在制造过程中不进行多温度校准的精密系统。此外，
// 非常低的失调电压和噪声允许在 A 至 kA 感测应用中使
// 用，并在感应分流器元件上提供宽的动态范围而不会产
// 生显著的功率损耗。该器件的低输入偏置电流允许使用
// 较大的电流检测电阻器，从而能够提供微安级的精确电
// 流测量。

namespace ymd::drivers{


class INA237 final:public INA237_Prelude{
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
                .bus_conv_time = ConversionTime::_150us,
                .shunt_conv_time = ConversionTime::_150us,
                .sample_res_mohms = 5, // 0.005ohms
                .max_current_ma = 10 * 1000 // 10A
            };
        }
    };


    explicit INA237(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit INA237(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit INA237(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, i2c_addr)){};

    INA237(const INA237 &) = delete;
    INA237(INA237 &&) = delete;
    ~INA237() = default;



    IResult<> init(const Config & cfg);

    IResult<> soft_reset();

    IResult<> validate();

    IResult<iq16> get_current();

    IResult<iq16> get_power();

    IResult<TemperatureCode> get_temperature();

    IResult<BusbarVoltageCode> get_busbar_voltage_code();

    IResult<ShuntValue> get_shunt_voltage_code();


    IResult<> enable_shunt_measure(const Enable en);

    IResult<> enable_busbar_measure(const Enable en);

    IResult<> enable_continuous_measure(const Enable en);

    IResult<> set_shunt_alert_threshold(
        const Option<ShuntValue> mi, 
        const Option<ShuntValue> ma);

    IResult<> set_busbar_alert_threshold(
        const Option<BusbarVoltageCode> mi, 
        const Option<BusbarVoltageCode> ma);

    IResult<> set_temperature_alert_threshold(const TemperatureCode ma);
private:
    hal::I2cDrv i2c_drv_;

    using Regset = INA237_Regs;
    Regset regs_ = {};
    
    iq16 current_lsb_ma_ = iq16(0.2);


    IResult<> set_average_times(const AverageTimes times);

    IResult<> set_busbar_conversion_time(const ConversionTime time);

    IResult<> set_shunt_conversion_time(const ConversionTime time);

    IResult<> set_scale(const uint32_t sample_res_mohms, const uint32_t max_current_ma);

    IResult<> write_reg(const RegAddr reg_addr, const uint16_t reg_val);

    IResult<> read_reg(const RegAddr reg_addr, uint16_t & reg_val);
    

    IResult<> write_bulk(const uint8_t reg_addr, const uint8_t *data, uint16_t size);
    IResult<> read_bulk(const uint8_t reg_addr, uint8_t *data, uint16_t size);

    template<typename T>
    IResult<> read_reg(T & reg){
        return read_reg(T::REG_ADDR, reg.as_bits_mut());
    }

    IResult<> read_reg(Regset::R24_Power & reg);
    
    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.commit_changes();
        return Ok();
    }
};
}