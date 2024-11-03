#pragma once

#include "hal/gpio/port_virtual.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"
#include "drivers/device_defs.h"

namespace yumud::drivers{
class PCA9685: public PortVirtualConcept<16>{
public:
    scexpr uint8_t default_i2c_addr = 0b10000000;
protected:
    I2cDrv bus_drv;

    scexpr uint8_t valid_chipid = 0x23;


    struct Mode1Reg:public Reg8{
        using Reg8::operator=;
        
        uint8_t allcall:1;
        uint8_t sub:3;
        uint8_t sleep:1;
        uint8_t auto_inc:1;
        uint8_t extclk:1;
        uint8_t restart:1;
    };

    struct Mode2Reg:public Reg8{
        using Reg8::operator=;

        uint8_t outne:2;
        uint8_t outdrv:1;
        uint8_t och:1;
        uint8_t invrt:1;
        uint8_t __resv__:3;
    };

    struct LedOnOffReg:public Reg16{
        uint16_t cvr:12 = 0;
        uint16_t full:1 = 0;
        uint16_t __resv__:3 = 0;
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

    Mode1Reg mode1_reg;
    Mode2Reg mode2_reg;
    std::array<uint8_t,3> sub_addr_regs;
    uint8_t all_addr_reg;
    std::array<LedRegs,16> sub_channels;
    LedRegs all_channel;
    uint8_t prescale_reg;

    class PCA8975Channel:public PwmChannel, GpioConcept{
    protected:
        PCA9685 & pca;
        uint8_t channel;

        PCA8975Channel(PCA9685 & _pca, const uint8_t _channel):GpioConcept(_channel), pca(_pca), channel(_channel){;}
        
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

        void setMode(const PinMode mode) override{}
    };


    std::array<PCA8975Channel,16> channels{
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
        PCA8975Channel{*this, 15}
    };

    __fast_inline void writeReg(const RegAddress addr, const uint8_t reg){
        bus_drv.writeReg((uint8_t)addr, reg, LSB);
    };

    __fast_inline void writeReg(const RegAddress addr, const uint16_t reg){
        bus_drv.writeReg((uint8_t)addr, reg, LSB);
    }

    __fast_inline void readReg(const RegAddress addr, uint8_t & reg){
        bus_drv.readReg((uint8_t)addr, reg, LSB);
    }

    __fast_inline void readReg(const RegAddress addr, uint16_t & reg){
        bus_drv.readReg((uint8_t)addr, reg, LSB);
    }

    uint8_t readReg(const RegAddress addr){
        uint8_t data;
        bus_drv.readReg((uint8_t)addr, data, LSB);
        return data;
    }

    void write(const uint16_t data) override{
        // buf = data;
        // writeReg(RegAddress::out, buf);
    }

    uint16_t read() override{
        // readReg(RegAddress::in, buf);
        // return buf;
        return true;
    }

    void test(){
        static_assert(sizeof(mode1_reg) == 1);
        static_assert(sizeof(mode2_reg) == 1);
    }
public:
    PCA9685(I2cDrv & _bus_drv):bus_drv(_bus_drv){;}
    PCA9685(I2cDrv && _bus_drv):bus_drv(_bus_drv){;}
    PCA9685(I2c & _bus):bus_drv{_bus, default_i2c_addr}{;}

    void setFrequency(const uint freq, const real_t trim = real_t(1));

    void setPwm(const uint8_t channel, const uint16_t on, const uint16_t off);

    void setSubAddr(const uint8_t index, const uint8_t addr);

    void enableExtClk(const bool en = true);

    void enableSleep(const bool en = true);

    void init();

    void reset();

    void set(const Pin pin) override;

    void clr(const Pin pin) override;

    void set(const uint16_t data) override;

    void clr(const uint16_t data) override;
    
    void writeByIndex(const int index, const bool data) override;

    bool readByIndex(const int index) override;

    void setMode(const int index, const PinMode mode) override;

    PCA9685 & operator = (const uint16_t data) override {write(data); return *this;}

    PCA8975Channel & operator [](const size_t index){
        return channels[index];
    }
};
};