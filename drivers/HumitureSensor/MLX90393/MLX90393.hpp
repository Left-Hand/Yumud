#pragma once

// 微小磁场传感器

// 迈来芯 MLX90393 是迈来芯位置传感器IC的新晋成员，
// 同时也是该产品组合中灵活性最高的最小组件。
// 此外，这款磁场传感器 IC 专为微功率应用而设计，
// 其占空比可在 0.1％ 至 100％ 的范围内编程设定。

// MIT license
// https://github.com/adafruit/MLX90393_Library/blob/master/MLX90393.h


// https://wiki.lckfb.com/zh-hans/lspi/module/sensor/mlx90393-3d-hall-sensor.html

#include "mlx90393_prelude.hpp"
#include "mlx90393_transport.hpp"


namespace ymd::drivers{

class MLX90393 final: public MLX90393_Prelude{
public:
    explicit MLX90393(MLX90393_TransportIntf & transport):
        transport_(transport){;}

    IResult<> init();
    IResult<> reset();
    IResult<> exit_mode();
    
    IResult<math::Vec3<iq24>> read_measurement();
    IResult<> start_single_measurement();
    
    IResult<> set_gain(Gain gain);
    
    IResult<> set_resolution(Axis, Resolution resolution);
    
    IResult<> set_filter(Filter filter);
    
    IResult<> set_oversampling(OverSampling oversampling);
    
    IResult<> enable_trig_interrupt(Enable en);
    IResult<math::Vec3<iq24>> read_data();

    
private:
    MLX90393_TransportIntf & transport_;
    IResult<> read_reg(uint8_t reg_addr, uint16_t & reg_val);
    IResult<> write_reg(uint8_t reg_addr, uint16_t reg_val);

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res =write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    IResult<> transceive(
        std::span<uint8_t> rx_pbuf, 
        std::span<const uint8_t> tx_pbuf, 
        const uint8_t interdelay
    ){
        return transport_.transceive(rx_pbuf, tx_pbuf);
    }

    MLX90393_Regset regs_;
};

}