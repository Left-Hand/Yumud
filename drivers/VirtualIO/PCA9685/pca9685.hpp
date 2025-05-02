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

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/errno.hpp"

#include "concept/pwm_channel.hpp"

#include "hal/gpio/vport.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{
class PCA9685 final: public hal::VGpioPortIntf<16>{
public:

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1000000);
    scexpr auto CHANNELS_COUNT = 16;

    enum class Error_Kind{
        IndexOutOfRange
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    struct Config{
        uint freq; 
        real_t trim = real_t(1);
    };


    class PCA9685Channel final:public hal::PwmIntf,  hal::GpioIntf{
    private:
        PCA9685 & pca_;
        uint8_t channel;

        PCA9685Channel(PCA9685 & _pca, const uint8_t _channel):
            pca_(_pca), channel(_channel){;}
        
        DELETE_COPY_AND_MOVE(PCA9685Channel)
        
        friend class PCA9685;
    public:

        PCA9685Channel & operator = (const real_t duty){
            pca_.set_pwm(channel, 0, uint16_t(duty << 12)).unwrap();
            return *this;
        }

        [[nodiscard]]
        __fast_inline Result<void, Error> set_duty(const real_t duty) {
            return pca_.set_pwm(channel, 0, uint16_t(duty << 12));
        }

        __fast_inline void set() {*this = real_t(1);}
        __fast_inline void clr() {*this = real_t(0);}
        __fast_inline void write(const BoolLevel val){
            *this = real_t(bool(val));
        }
        BoolLevel read() const;

        __fast_inline int8_t index() const {return channel;}

        void set_mode(const hal::GpioMode mode){}
    };
private:
    hal::I2cDrv i2c_drv_;

    scexpr uint8_t valid_chipid = 0x23;
    struct Mode1Reg:public Reg8<>{
        using Reg8::operator=;
        
        uint8_t allcall:1;
        uint8_t sub:3;
        uint8_t sleep:1;
        uint8_t auto_inc:1;
        uint8_t extclk:1;
        uint8_t restart:1;
    };

    struct Mode2Reg:public Reg8<>{
        using Reg8::operator=;

        uint8_t outne:2;
        uint8_t outdrv:1;
        uint8_t och:1;
        uint8_t invrt:1;
        uint8_t __resv__:3;
    };

    struct LedOnOffReg:public Reg16<>{
        uint16_t cvr:12 = 0;
        uint16_t full:1 = 0;
        const uint16_t __resv__:3 = 0;
    };

    static_assert(sizeof(Mode1Reg) == 1);
    static_assert(sizeof(Mode2Reg) == 1);

    struct LedRegs{
        LedOnOffReg  on;
        LedOnOffReg off;
    };

    enum class RegAddress:uint8_t{
        Mode1,
        Mode2,
        LED0_ON_L = 0x06,
        LED0_ON_H,
        LED0_OFF_L,
        LED0_OFF_H,
        SubAddr = 0x02,
        Prescale = 0xfe
    };

    Mode1Reg mode1_reg = {};
    Mode2Reg mode2_reg = {};
    std::array<uint8_t,3> sub_addr_regs = {};
    uint8_t all_addr_reg = {};
    std::array<LedRegs, CHANNELS_COUNT> sub_channels = {};
    LedRegs all_channel = {};
    uint8_t prescale_reg = {};

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


    void write_mask(const uint16_t data);
    uint16_t read_mask();

    [[nodiscard]] Result<void, Error> init();
    [[nodiscard]] Result<void, Error> validate(){
        const auto res = i2c_drv_.validate();
        if(res.is_err()) return Err(Error::HalError(res.unwrap_err()));
        return Ok();
    }
public:
    PCA9685(hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    PCA9685(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    PCA9685(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_{i2c, addr}{;}

    [[nodiscard]] Result<void, Error> set_frequency(const uint freq, const real_t trim);

    [[nodiscard]] Result<void, Error> set_pwm(const uint8_t channel, const uint16_t on, const uint16_t off);

    [[nodiscard]] Result<void, Error> set_sub_addr(const uint8_t index, const uint8_t addr);

    [[nodiscard]] Result<void, Error> enable_ext_clk(const bool en = true);

    [[nodiscard]] Result<void, Error> enable_sleep(const bool en = true);

    [[nodiscard]] Result<void, Error> init(const Config & cfg){
        return init() | reconf(cfg);
    }
    [[nodiscard]] Result<void, Error> reconf(const Config & cfg){
        return set_frequency(cfg.freq, cfg.trim);
    }

    [[nodiscard]] Result<void, Error> reset();

    void set_by_mask(const uint16_t data);

    void clr_by_mask(const uint16_t data);

    void write_by_mask(const uint16_t data);
    
    void write_by_index(const size_t index, const BoolLevel data);

    BoolLevel read_by_index(const size_t index);

    void set_mode(const size_t index, const hal::GpioMode mode);

    template<typename ...Args>
    requires (std::is_integral_v<Args> && ...)
    std::array<uint16_t, sizeof...(Args)> dump_cvr(Args ...args){
        auto dump_one = [&](const uint idx) -> uint16_t{
            if(idx >= CHANNELS_COUNT) __builtin_abort();
            return sub_channels[idx].off.cvr;
        };

        return {dump_one(args)...};
    }

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
    
    PCA9685Channel & operator [](const size_t index){
        if(index >= CHANNELS_COUNT) __builtin_abort();
        return channels[index];
    }
};
};