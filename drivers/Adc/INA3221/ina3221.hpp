#pragma once


//这个驱动已经完成
//这个驱动已经测试
//这个驱动还未支持所有特性

// eg:
// INA3221 ina = {&i2c};

// ina.init().unwrap();

// while(true){
//     ina.update(INA3221::ChannelSelection::CH1).unwrap();
//     DEBUG_PRINTLN(
//         ina.get_bus_volt(ch).unwrap(), 
//         ina.get_shunt_volt(ch).unwrap() * iq16(INV_SHUNT_RES)
//     );
//     clock::delay(2ms);
// }



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
    public INA3221_Regs{
public:

    explicit INA3221(const hal::I2cDrv & i2c_drv):
        phy_(i2c_drv){;}
    explicit INA3221(hal::I2cDrv && i2c_drv):
        phy_(std::move(i2c_drv)){;}
    explicit INA3221(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv(i2c, addr)){;}
    ~INA3221(){;}
    
    [[nodiscard]] IResult<bool> is_ready();


    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> reconf(const Config & cfg);
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> update(const ChannelSelection sel);
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<> set_average_times(const AverageTimes times);
    [[nodiscard]] IResult<> enable_measure_bus(const Enable en);
    [[nodiscard]] IResult<> enable_measure_shunt(const Enable en);
    [[nodiscard]] IResult<> enable_continuous(const Enable en);

    [[nodiscard]] IResult<> enable_channel(const ChannelSelection sel, const Enable en);

    [[nodiscard]] IResult<> set_bus_conversion_time(const ConversionTime time);
    [[nodiscard]] IResult<> set_shunt_conversion_time(const ConversionTime time);

    [[nodiscard]] IResult<int> get_shunt_volt_uv(const ChannelSelection sel);
    [[nodiscard]] IResult<int> get_bus_volt_mv(const ChannelSelection sel);

    [[nodiscard]] IResult<iq16> get_shunt_volt(const ChannelSelection sel);
    [[nodiscard]] IResult<iq16> get_bus_volt(const ChannelSelection sel);

    [[nodiscard]] IResult<> set_instant_ovc_threshold(const ChannelSelection sel, const iq16 volt);
    [[nodiscard]] IResult<> set_constant_ovc_threshold(const ChannelSelection sel, const iq16 volt);
private:
    INA3221_Phy phy_;


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = phy_.write_reg(T::ADDRESS, reg.to_bits());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return phy_.read_reg(T::ADDRESS, reg.as_bits_mut());
    }

    [[nodiscard]] IResult<> read_reg(const RegAddr addr, auto & data){
        return phy_.read_reg(addr, data);
    }

    [[nodiscard]] IResult<> write_reg(const RegAddr addr, const auto data){
        return phy_.write_reg(addr, data);
    }

};

}