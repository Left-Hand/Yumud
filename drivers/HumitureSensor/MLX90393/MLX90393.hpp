#pragma once

// 迈来芯 MLX90393 是迈来芯位置传感器IC的新晋成员，
// 同时也是该产品组合中灵活性最高的最小组件。此外，这款磁场传感器 IC 专为微功率应用而设计，
// 其占空比可在 0.1％ 至 100％ 的范围内编程设定。

// https://github.com/adafruit/MLX90393_Library/blob/master/MLX90393.h

#include "mlx90393_prelude.hpp"
namespace ymd::drivers{


class MLX90393 final: public MLX90393_Prelude{
public:
    explicit MLX90393(MLX90393_Phy && phy):
        phy_(phy){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<> exit_mode();
    
    [[nodiscard]] IResult<Vec3<q24>> read_measurement();
    [[nodiscard]] IResult<> start_single_measurement();
    
    [[nodiscard]] IResult<> set_gain(Gain gain);
    
    [[nodiscard]] IResult<> set_resolution(Axis, Resolution resolution);
    
    [[nodiscard]] IResult<> set_filter(Filter filter);
    
    [[nodiscard]] IResult<> set_oversampling(OverSampling oversampling);
    
    [[nodiscard]] IResult<> enable_trig_int(Enable en);
    [[nodiscard]] IResult<Vec3<q24>> read_data();

    
private:
    MLX90393_Phy phy_;
    [[nodiscard]] IResult<> read_register(uint8_t reg, uint16_t & data);
    [[nodiscard]] IResult<> write_register(uint8_t reg, uint16_t data);

    template<typename T>
    [[nodiscard]] IResult<> write_register(const RegCopy<T> & reg){
        if(const auto res =write_register(T::ADDRESS, reg.as_val());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    [[nodiscard]] IResult<> transceive(
        std::span<uint8_t> rx_pbuf, 
        std::span<const uint8_t> tx_pbuf, 
        const uint8_t interdelay
    ){
        return phy_.transceive(rx_pbuf, tx_pbuf);
    }

    MLX90393_Regset regs_;
};

}