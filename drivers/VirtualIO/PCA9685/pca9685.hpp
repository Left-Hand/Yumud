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

#include "details/PCA9685_collections.hpp"

namespace ymd::drivers{
class PCA9685 final:public PCA9685_Regs{
public:
    class PCA9685Channel;
    class PCA9685_Vport;
    PCA9685(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    PCA9685(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    PCA9685(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_{i2c, addr}{;}

    [[nodiscard]] Result<void, Error> set_frequency(const uint freq, const real_t trim);

    [[nodiscard]] Result<void, Error> set_pwm(const uint8_t channel, const uint16_t on, const uint16_t off);

    [[nodiscard]] Result<void, Error> set_sub_addr(const uint8_t index, const uint8_t addr);

    [[nodiscard]] Result<void, Error> enable_ext_clk(const Enable en = EN);

    [[nodiscard]] Result<void, Error> enable_sleep(const Enable en = EN);

    [[nodiscard]] Result<void, Error> init(const Config & cfg){
        return init() | reconf(cfg);
    }
    [[nodiscard]] Result<void, Error> reconf(const Config & cfg){
        return set_frequency(cfg.freq, cfg.trim);
    }

    [[nodiscard]] Result<void, Error> reset();



    template<typename ...Args>
    requires (std::is_integral_v<Args> && ...)
    constexpr std::array<uint16_t, sizeof...(Args)> dump_cvr(Args ...args){
        auto dump_one = [&](const uint idx) -> uint16_t{
            if(idx >= CHANNELS_COUNT) sys::abort();
            return sub_channels[idx].off.cvr;
        };

        return {dump_one(args)...};
    }
    
    PCA9685Channel & operator [](const size_t index){
        if(index >= CHANNELS_COUNT) sys::abort();
        return channels[index];
    }

    class PCA9685_Vport final: public hal::VGpioPortIntf<16>{ 
        void write_mask(const hal::PinMask mask);
        hal::PinMask read_mask();

        void set_by_mask(const hal::PinMask mask);

        void clr_by_mask(const hal::PinMask mask);
    
        void write_by_mask(const hal::PinMask mask);
        
        void write_by_index(const size_t index, const BoolLevel data);
    
        BoolLevel read_by_index(const size_t index);
    
        void set_mode(const size_t index, const hal::GpioMode mode);
    };

    class PCA9685Channel final:public hal::PwmIntf,  hal::GpioIntf{
    private:
        PCA9685 & pca_;
        uint8_t channel_;

        PCA9685Channel(PCA9685 & _pca, const uint8_t channel):
            pca_(_pca), channel_(channel){;}
        
        PCA9685Channel(const PCA9685Channel & other) = delete;
        PCA9685Channel(PCA9685Channel && other) = delete;
        

        friend class PCA9685;
    public:

        void set_duty(const real_t duty){
            pca_.set_pwm(channel_, 0, uint16_t(duty << 12)).unwrap();
        }

        // [[nodiscard]]
        // __fast_inline Result<void, Error> set_duty(const real_t duty) {
        //     return pca_.set_pwm(channel_, 0, uint16_t(duty << 12));
        // }

        __fast_inline void set() {this->set_duty(real_t(1));}
        __fast_inline void clr() {this->set_duty(real_t(0));}
        __fast_inline void write(const BoolLevel val){
            this->set_duty(real_t((val.to_bool())));
        }

        BoolLevel read() const;

        __fast_inline int8_t index() const {return channel_;}

        void set_mode(const hal::GpioMode mode){}
    };
private:
    scexpr uint8_t VALID_CHIP_ID = 0x23;

    hal::I2cDrv i2c_drv_;

    std::array<PCA9685Channel, 16> channels ={
        PCA9685Channel{*this, 0},
        PCA9685Channel{*this, 1},
        PCA9685Channel{*this, 2},
        PCA9685Channel{*this, 3},
        PCA9685Channel{*this, 4},
        PCA9685Channel{*this, 5},
        PCA9685Channel{*this, 6},
        PCA9685Channel{*this, 7},
        PCA9685Channel{*this, 8},
        PCA9685Channel{*this, 9},
        PCA9685Channel{*this, 10},
        PCA9685Channel{*this, 11},
        PCA9685Channel{*this, 12},
        PCA9685Channel{*this, 13},
        PCA9685Channel{*this, 14},
        PCA9685Channel{*this, 15},
    };


    [[nodiscard]] Result<void, Error> write_reg(const RegAddress addr, const uint8_t reg){
        const auto res = i2c_drv_.write_reg(uint8_t(addr), reg);
        if(res.is_err()) return Err(Error::HalError(res.unwrap_err()));
        return Ok();
    };

    [[nodiscard]] Result<void, Error> write_reg(const RegAddress addr, const uint16_t reg){
        const auto res = i2c_drv_.write_reg(uint8_t(addr), reg, LSB);
        if(res.is_err()) return Err(Error::HalError(res.unwrap_err()));
        return Ok();
    }

    [[nodiscard]] Result<void, Error> read_reg(const RegAddress addr, uint8_t & reg){
        const auto res = i2c_drv_.read_reg(uint8_t(addr), reg);
        if(res.is_err()) return Err(Error::HalError(res.unwrap_err()));
        return Ok();
    }

    [[nodiscard]] Result<void, Error> read_reg(const RegAddress addr, uint16_t & reg){
        const auto res = i2c_drv_.read_reg(uint8_t(addr), reg, LSB);
        if(res.is_err()) return Err(Error::HalError(res.unwrap_err()));
        return Ok();
    }

    [[nodiscard]] Result<void, Error> init();
    [[nodiscard]] Result<void, Error> validate();
};

// static constexpr auto a = sizeof(PCA9685::PCA9685_Regs);
// static constexpr auto a = sizeof(PCA9685::PCA9685Channel);
};