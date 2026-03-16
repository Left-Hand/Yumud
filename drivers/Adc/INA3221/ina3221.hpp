#pragma once


//这个驱动已经完成
//这个驱动已经测试
//这个驱动还未支持所有特性


// 注意：
// 1. INA3221不支持地址自增模式，每次对寄存器的操作都是单独的

// INA3221 是一款三通道、高侧电流和总线电压监视
// 器，具有一个兼容I2C和SMBUS的接口。INA3221
// 不仅能够监视分流压降和总线电源电压，还针对这些信
// 号提供有可编程的转换时间和平均值计算模式。
// INA3221 提供关键报警和警告报警，用于检测每条通
// 道上可编程的多种超范围情况。

#include "ina3221_prelude.hpp"

namespace ymd::drivers{


class INA3221 final:
    public INA3221_Prelude{
public:

    explicit INA3221(const hal::I2cDrv & i2c_drv):
        transport_(i2c_drv){;}
    explicit INA3221(hal::I2cDrv && i2c_drv):
        transport_(std::move(i2c_drv)){;}
    explicit INA3221(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        transport_(hal::I2cDrv(i2c, i2c_addr)){;}

    INA3221(const INA3221 &) = delete;
    INA3221(INA3221 &&) = delete;

    ~INA3221(){;}

    IResult<bool> is_ready();


    IResult<> init(const Config & cfg);
    IResult<> reconf(const Config & cfg);
    IResult<> update(const ChannelSelection ch_sel);
    IResult<> validate();
    IResult<> reset();
    IResult<> set_average_times(const AverageTimes times);
    IResult<> enable_measure_bus(const Enable en);
    IResult<> enable_measure_shunt(const Enable en);
    IResult<> enable_continuous(const Enable en);

    IResult<> enable_channel(const ChannelSelection ch_sel, const Enable en);

    IResult<> set_bus_conversion_time(const ConversionTime time);
    IResult<> set_shunt_conversion_time(const ConversionTime time);

    IResult<ShuntVoltCode> get_shunt_volt_code(const ChannelSelection ch_sel);
    IResult<BusVoltCode> get_bus_volt_code(const ChannelSelection ch_sel);

    IResult<> set_instant_ovc_threshold(const ChannelSelection ch_sel, const ShuntVoltCode volt_code);
    IResult<> set_constant_ovc_threshold(const ChannelSelection ch_sel, const ShuntVoltCode volt_code);
private:
    INA3221_Transport transport_;
    INA3221_Regs regs_ = {};

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = transport_.write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        return transport_.read_reg(T::REG_ADDR, reg.as_bits_mut());
    }

    IResult<> read_reg(const RegAddr reg_addr, auto & reg_data){
        return transport_.read_reg(reg_addr, reg_data);
    }

    IResult<> write_reg(const RegAddr reg_addr, const auto reg_data){
        return transport_.write_reg(reg_addr, reg_data);
    }

};

}