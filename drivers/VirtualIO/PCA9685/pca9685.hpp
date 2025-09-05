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
class PCA9685 final:public PCA9685_Regs{
public:
    class PCA9685Channel;
    class PCA9685_Vport;
    PCA9685(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    PCA9685(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    PCA9685(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_{i2c, addr}{;}

    [[nodiscard]] IResult<> init(const Config & cfg){
        if(const auto res = init(); res.is_err()) return Err(res.unwrap_err());
        if(const auto res = reconf(cfg); res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
    [[nodiscard]] IResult<> reconf(const Config & cfg){
        return set_frequency(cfg.freq, cfg.trim);
    }

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> validate();
    
    [[nodiscard]] IResult<> set_frequency(const uint freq, const real_t trim);

    [[nodiscard]] IResult<> set_pwm(const Nth nth, const uint16_t on, const uint16_t off);

    [[nodiscard]] IResult<> set_sub_addr(const uint8_t index, const uint8_t addr);

    [[nodiscard]] IResult<> enable_ext_clk(const Enable en = EN);

    [[nodiscard]] IResult<> enable_sleep(const Enable en = EN);




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
        
        void write_nth(const size_t index, const BoolLevel data);
    
        BoolLevel read_nth(const size_t index);
    
        void set_mode(const size_t index, const hal::GpioMode mode);
    };

    class PCA9685Channel final:public hal::PwmIntf,  hal::GpioIntf{
    private:
        PCA9685 & pca_;
        Nth nth_;

        PCA9685Channel(PCA9685 & _pca, const Nth nth):
            pca_(_pca), nth_(nth){;}
        
        PCA9685Channel(const PCA9685Channel & other) = delete;
        PCA9685Channel(PCA9685Channel && other) = delete;
        

        friend class PCA9685;
    public:

        void set_dutycycle(const real_t duty){
            pca_.set_pwm(nth_, 0, uint16_t(duty << 12)).unwrap();
        }
        __fast_inline void set() {this->set_dutycycle(real_t(1));}
        __fast_inline void clr() {this->set_dutycycle(real_t(0));}
        __fast_inline void write(const BoolLevel val){
            this->set_dutycycle(real_t((val.to_bool())));
        }

        BoolLevel read() const;

        __fast_inline Nth nth() const {return nth_;}

        void set_mode(const hal::GpioMode mode){}
    };
private:
    static constexpr uint8_t VALID_CHIP_ID = 0x23;

    hal::I2cDrv i2c_drv_;

    std::array<PCA9685Channel, 16> channels ={
        PCA9685Channel{*this, 0_nth},
        PCA9685Channel{*this, 1_nth},
        PCA9685Channel{*this, 2_nth},
        PCA9685Channel{*this, 3_nth},
        PCA9685Channel{*this, 4_nth},
        PCA9685Channel{*this, 5_nth},
        PCA9685Channel{*this, 6_nth},
        PCA9685Channel{*this, 7_nth},
        PCA9685Channel{*this, 8_nth},
        PCA9685Channel{*this, 9_nth},
        PCA9685Channel{*this, 10_nth},
        PCA9685Channel{*this, 11_nth},
        PCA9685Channel{*this, 12_nth},
        PCA9685Channel{*this, 13_nth},
        PCA9685Channel{*this, 14_nth},
        PCA9685Channel{*this, 15_nth},
    };


    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const uint8_t reg){
        const auto res = i2c_drv_.write_reg(uint8_t(addr), reg);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    };

    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const uint16_t reg){
        const auto res = i2c_drv_.write_reg(uint8_t(addr), reg, LSB);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


    [[nodiscard]] IResult<> read_reg(const RegAddress addr, uint8_t & reg){
        const auto res = i2c_drv_.read_reg(uint8_t(addr), reg);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const RegAddress addr, uint16_t & reg){
        const auto res = i2c_drv_.read_reg(uint8_t(addr), reg, LSB);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.as_val()); 
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    };

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = read_reg(T::ADDRESS, reg.as_ref()); 
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    };


};
};