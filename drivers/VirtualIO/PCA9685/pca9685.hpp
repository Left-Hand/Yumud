#pragma once

#include "core/io/regs.hpp"

#include "hal/gpio/vport.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "concept/pwm_channel.hpp"

namespace ymd::drivers{
class PCA9685 final: public hal::VGpioPortIntf<16>{
public:

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b10000000);


    class PCA9685Channel final:public hal::PwmIntf,  hal::GpioIntf{
    protected:
        PCA9685 & pca;
        uint8_t channel;

        PCA9685Channel(PCA9685 & _pca, const uint8_t _channel):
            pca(_pca), channel(_channel){;}
        
        DELETE_COPY_AND_MOVE(PCA9685Channel)
        
        friend class PCA9685;
    public:

        PCA9685Channel & operator = (const real_t duty) override{
            pca.set_pwm(channel, 0, (uint16_t)(duty << 12));
            return *this;
        }

        __fast_inline void set() override {*this = real_t(1);}
        __fast_inline void clr() override {*this = real_t(0);}
        __fast_inline void write(const BoolLevel val){
            this->set_duty(real_t(bool(val)));
        }
        BoolLevel read() const override;

        __fast_inline int8_t index() const {return channel;}

        void set_mode(const hal::GpioMode mode) override{}
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

    hal::BusError write_reg(const RegAddress addr, const uint8_t reg){
        return i2c_drv_.write_reg(uint8_t(addr), reg);
    };

    hal::BusError write_reg(const RegAddress addr, const uint16_t reg){
        return i2c_drv_.write_reg(uint8_t(addr), reg, LSB);
    }

    hal::BusError read_reg(const RegAddress addr, uint8_t & reg){
        return i2c_drv_.read_reg(uint8_t(addr), reg);
    }

    hal::BusError read_reg(const RegAddress addr, uint16_t & reg){
        return i2c_drv_.read_reg(uint8_t(addr), reg, LSB);
    }
    void write_mask(const uint16_t data);
    uint16_t read_mask();

    void test(){
        static_assert(sizeof(mode1_reg) == 1);
        static_assert(sizeof(mode2_reg) == 1);
    }
public:
    PCA9685(hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    PCA9685(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    PCA9685(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_{i2c, addr}{;}

    void set_frequency(const uint freq, const real_t trim = real_t(1));

    void set_pwm(const uint8_t channel, const uint16_t on, const uint16_t off);

    void set_sub_addr(const uint8_t index, const uint8_t addr);

    void enable_ext_clk(const bool en = true);

    void enable_sleep(const bool en = true);

    void init(const uint freq, const real_t trim){
        init();
        set_frequency(freq, trim);
    }

    void init();

    void reset();

    void set_by_mask(const uint16_t data) override;

    void clr_by_mask(const uint16_t data) override;
    
    void write_by_index(const size_t index, const BoolLevel data) override;

    BoolLevel read_by_index(const size_t index) override;

    void set_mode(const size_t index, const hal::GpioMode mode) override;

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

        if(index >= 16) HALT;
        return channels[index];
    }
};
};