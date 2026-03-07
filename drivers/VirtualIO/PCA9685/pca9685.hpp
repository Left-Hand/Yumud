#pragma once

// 这个驱动已经完成
// 这个驱动已经测试

// I2C设备 地址: 0b1xxxxxx 最高速率 400KHz

// 官方介绍: https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf
// PCA9685是一款由I²C总线控制的16通道LED控制器，
// 适用于红色/绿色/蓝色/琥珀色(RGBA)彩色背光应用。
// 每个LED输出各自带有12位分辨率(4096级)固定频率的独立PWM控制器，
// 该控制器工作在24Hz至1526Hz的可编程频率范围内，占空比从0%至100%可调，
// 允许将LED设为特定的亮度值。所有输出均设为相同的PWM频率。

#include "details/PCA9685_Prelude.hpp"

namespace ymd::drivers{
class PCA9685 final:public PCA9685_Prelude{
public:
    class PCA9685Channel;
    class PCA9685_Vport;
    explicit PCA9685(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    explicit PCA9685(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    explicit PCA9685(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_{i2c, i2c_addr}{;}

    IResult<> init(const Config & cfg){
        if(const auto res = init(); res.is_err()) return Err(res.unwrap_err());
        if(const auto res = reconf(cfg); res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
    IResult<> reconf(const Config & cfg){
        return set_frequency(cfg.freq, cfg.trim);
    }

    IResult<> reset();

    IResult<> init();

    IResult<> validate();
    
    IResult<> set_frequency(const uint32_t freq, const iq16 trim);

    IResult<> set_pwm(const Nth nth, const uint16_t on, const uint16_t off);

    IResult<> set_sub_addr(const uint8_t index, const uint8_t addr);

    IResult<> enable_ext_clk(const Enable en);

    IResult<> enable_sleep(const Enable en);


    template<typename ...Args>
    requires (std::is_integral_v<Args> && ...)
    constexpr std::array<uint16_t, sizeof...(Args)> dump_cvr(Args ...args){
        auto dump_one = [&](const uint idx) -> uint16_t{
            if(idx >= CHANNELS_COUNT) sys::abort();
            return regs_.sub_channels[idx].off.cvr;
        };

        return {dump_one(args)...};
    }
    

private:

    hal::I2cDrv i2c_drv_;
    PCA9685_Regset regs_ = {};

    IResult<> write_reg(const RegAddr reg_addr, const uint8_t reg_val){
        const auto res = i2c_drv_.write_reg(uint8_t(reg_addr), reg_val);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    };

    IResult<> write_reg(const RegAddr reg_addr, const uint16_t reg_val){
        const auto res = i2c_drv_.write_reg(uint8_t(reg_addr), reg_val, std::endian::little);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


    IResult<> read_reg(const RegAddr reg_addr, uint8_t & reg_val){
        const auto res = i2c_drv_.read_reg(uint8_t(reg_addr), reg_val);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(const RegAddr reg_addr, uint16_t & reg_val){
        const auto res = i2c_drv_.read_reg(uint8_t(reg_addr), reg_val, std::endian::little);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::REG_ADDR, reg.to_bits()); 
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    };

    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto res = read_reg(T::REG_ADDR, reg.as_bits_mut()); 
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    };


};
};