#pragma once

#include "tcs34725_prelude.hpp"

namespace ymd::drivers{


class TCS34725 final:public TCS34725_Regs{
public:
    struct Config{
        Milliseconds integration_time = 240ms;
        Gain gain = Gain::_1x;
    };

    explicit TCS34725(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit TCS34725(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit TCS34725(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(i2c, addr){;}

    TCS34725(const TCS34725 &) = delete;
    TCS34725(TCS34725 &&) = delete;
    ~TCS34725() = default;

        
    IResult<> init(const Config & cfg);

    IResult<> validate();

    IResult<> set_integration_time(const Milliseconds ms);

    IResult<> set_wait_time(const Milliseconds ms);

    IResult<> set_int_thr_low(const uint16_t thr);

    IResult<> set_int_thr_high(const uint16_t thr);

    IResult<> set_int_persistence(const uint8_t times);

    IResult<> set_gain(const Gain gain);

    IResult<uint8_t> get_id();
    IResult<bool> is_idle();

    IResult<> set_power(const bool on);
    IResult<> start_conv();
    IResult<> update();

    std::tuple<real_t, real_t, real_t, real_t> get_crgb();

private:
    hal::I2cDrv i2c_drv_;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(
            conv_reg_address_repeated(T::ADDRESS), 
            reg.as_val(), LSB);
        res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(
            conv_reg_address_repeated(T::ADDRESS), reg.as_ref(), LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const RegAddr addr, const std::span<uint16_t> pbuf);

    static constexpr uint8_t conv_reg_address_norepeat(const RegAddr addr){
        return (std::bit_cast<uint8_t>(addr) | 0x80);
    }

    static constexpr uint8_t conv_reg_address_repeated(const RegAddr addr){
        return conv_reg_address_norepeat(addr) | (1 << 5);
    }

};

};