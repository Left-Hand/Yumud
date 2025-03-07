#pragma once

#include "hal/gpio/port_virtual.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"
#include "drivers/device_defs.h"

namespace ymd::drivers{
class PCA9685 final: public hal::VGpioPortIntf<16>{
public:
    scexpr uint8_t default_i2c_addr = 0b10000000;
protected:
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
    std::array<LedRegs,16> sub_channels = {};
    LedRegs all_channel = {};
    uint8_t prescale_reg = {};

    class PCA8975Channel final:public hal::PwmIntf,  hal::GpioIntf{
    protected:
        PCA9685 & pca;
        uint8_t channel;

        PCA8975Channel(PCA9685 & _pca, const uint8_t _channel):
            pca(_pca), channel(_channel){;}
        
        DELETE_COPY_AND_MOVE(PCA8975Channel)
        
        friend class PCA9685;
    public:

        PCA8975Channel & operator = (const real_t duty) override{
            pca.setPwm(channel, 0, (uint16_t)(duty << 12));
            return *this;
        }

        __fast_inline void set() override {*this = real_t(1);}
        __fast_inline void clr() override {*this = real_t(0);}
        __fast_inline void write(const bool val){*this = real_t(val);}
        __fast_inline bool read() const override {return 0;}

        __fast_inline int8_t index() const {return channel;}

        void setMode(const hal::GpioMode mode) override{}
    };

    __fast_inline void writeReg(const RegAddress addr, const uint8_t reg){
        i2c_drv_.writeReg((uint8_t)addr, reg);
    };

    __fast_inline void writeReg(const RegAddress addr, const uint16_t reg){
        i2c_drv_.writeReg((uint8_t)addr, reg, LSB);
    }

    __fast_inline void readReg(const RegAddress addr, uint8_t & reg){
        i2c_drv_.readReg((uint8_t)addr, reg);
    }

    __fast_inline void readReg(const RegAddress addr, uint16_t & reg){
        i2c_drv_.readReg((uint8_t)addr, reg, LSB);
    }

    uint8_t readReg(const RegAddress addr){
        uint8_t data;
        i2c_drv_.readReg((uint8_t)addr, data);
        return data;
    }
    void writePort(const uint16_t data) override{
    }

    uint16_t readPort() override{
        return true;
    }

    void test(){
        static_assert(sizeof(mode1_reg) == 1);
        static_assert(sizeof(mode2_reg) == 1);
    }
public:
    PCA9685(hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    PCA9685(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    PCA9685(hal::I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_{i2c, i2c_addr}{;}

    void setFrequency(const uint freq, const real_t trim = real_t(1));

    void setPwm(const uint8_t channel, const uint16_t on, const uint16_t off);

    void setSubAddr(const uint8_t index, const uint8_t addr);

    void enableExtClk(const bool en = true);

    void enableSleep(const bool en = true);

    void init(const uint freq, const real_t trim){
        init();
        setFrequency(freq, trim);
    }

    void init();

    void reset();

    void setPin(const uint16_t data) override;

    void clrPin(const uint16_t data) override;
    
    void writeByIndex(const int index, const bool data) override;

    bool readByIndex(const int index) override;

    void setMode(const int index, const hal::GpioMode mode) override;

    PCA9685 & operator = (const uint16_t data) override {writePort(data); return *this;}

    std::array<PCA8975Channel, 16> channels ={
        PCA8975Channel{*this, 0},
        PCA8975Channel{*this, 1},
        PCA8975Channel{*this, 2},
        PCA8975Channel{*this, 3},
        PCA8975Channel{*this, 4},
        PCA8975Channel{*this, 5},
        PCA8975Channel{*this, 6},
        PCA8975Channel{*this, 7},
        PCA8975Channel{*this, 8},
        PCA8975Channel{*this, 9},
        PCA8975Channel{*this, 10},
        PCA8975Channel{*this, 11},
        PCA8975Channel{*this, 12},
        PCA8975Channel{*this, 13},
        PCA8975Channel{*this, 14},
        PCA8975Channel{*this, 15},
    };
    
    PCA8975Channel & operator [](const size_t index){

        if(index >= 16) HALT;
        return channels[index];
    }
};
};