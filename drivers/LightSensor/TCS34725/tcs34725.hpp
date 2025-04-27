#pragma once

#include <tuple>

#include "core/io/regs.hpp"
#include "core/math/real.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
namespace ymd::drivers{

class TCS34725{
public:
    enum class Gain:uint8_t{
        X1 = 0, X4, X16, X60 
    };

protected:
    hal::I2cDrv i2c_drv_;

    struct EnableReg:public Reg8<>{
        uint8_t powerOn : 1;
        uint8_t adcEn : 1;
        uint8_t __resv1__ :2;
        uint8_t waitEn : 1;
        uint8_t intEn : 1;
        uint8_t __resv2__ :3;
    };

    struct IntPersistenceReg:public Reg8<>{
        using Reg8::operator=;
        uint8_t __resv__ :4;
        uint8_t apers   :4;
    };

    struct LongWaitReg:public Reg8<>{
        uint8_t __resv1__ :1;
        uint8_t waitLong : 1;
        uint8_t __resv2__ :6;
    };

    struct GainReg:public Reg8<>{
        using Reg8::operator=;
        uint8_t gain        :2;
        uint8_t __resv2__   :6;
    };

    struct StatusReg:public Reg8<>{
        uint8_t done_flag    :1;
        uint8_t __resv1__   :3;
        uint8_t interrupt_flag     :1;
        uint8_t __resv2__   :3;
    };


    EnableReg enableReg;
    uint8_t integrationReg;
    uint8_t waitTimeReg;
    uint16_t lowThrReg;
    uint16_t highThrReg;
    IntPersistenceReg intPersistenceReg;
    LongWaitReg longWaitReg;
    GainReg gainReg;
    uint8_t deviceIdReg;
    StatusReg statusReg;
    uint16_t crgb[4] = {0};

    enum class RegAddress:uint8_t{
        Enable = 0x00,
        Integration = 0x01,
        WaitTime = 0x03,
        LowThr = 0x04,
        HighThr = 0x06,
        IntPersistence = 0x0C,
        LongWait = 0x0D,
        Gain = 0x0F,
        DeviceId = 0x12,
        Status = 0x13,
        ClearData = 0x14,
        RedData = 0x16,
        GreenData = 0x18,
        BlueData = 0x1A
    };

    uint8_t conv_reg_address(const RegAddress addr, bool repeat = true){
        return ((uint8_t) addr) | 0x80 | (repeat ? 1 << 5 : 0);
    }

    hal::BusError write_reg(const RegAddress addr, const uint16_t data){
        return i2c_drv_.write_reg(conv_reg_address(addr), (uint16_t)data, LSB);
    }

    hal::BusError read_reg(const RegAddress addr, uint16_t & data){
        return i2c_drv_.read_reg(conv_reg_address(addr), (uint16_t &)data, LSB);
    }

    hal::BusError write_reg(const RegAddress addr, const uint8_t data){
        return i2c_drv_.write_reg(conv_reg_address(addr, false), (uint8_t)data);
    }

    hal::BusError read_reg(const RegAddress addr, uint8_t & data){
        return i2c_drv_.read_reg(conv_reg_address(addr, false), (uint8_t &)data);
    }

    hal::BusError request_reg_data(const RegAddress addr, uint16_t * data_ptr, const size_t len);

public:
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x52);

    TCS34725(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    TCS34725(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    TCS34725(hal::I2c & bus, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(bus, addr){;}

    void set_integration(const uint16_t ms){
        uint16_t cycles = CLAMP(ms * 10 / 24, 1, 256);
        uint8_t temp = 256 - cycles;
        integrationReg = temp;
        write_reg(RegAddress::Integration, integrationReg);
    }

    void set_wait_time(const uint16_t ms){
        uint16_t ms_l = MAX(ms * 10 / 24,1);
        uint16_t value;
        bool long_waitFlag = false;
        if(ms_l <= 256){
            value = 256 - ms_l;
        }else{
            uint16_t ms_h = CLAMP(ms * 10 / 24 / 12, 1, 256);
            value = 256 - ms_h;
            long_waitFlag = true;
        }

        waitTimeReg = value;
        write_reg(RegAddress::WaitTime, waitTimeReg);
        if(long_waitFlag){
            longWaitReg.waitLong = true;
            write_reg(RegAddress::LongWait, longWaitReg);
        }
    }

    void set_int_thr_low(const uint16_t thr){
        lowThrReg = thr;
        write_reg(RegAddress::LowThr, lowThrReg);
    }

    void set_int_thr_high(const uint16_t thr){
        highThrReg = thr;
        write_reg(RegAddress::HighThr, highThrReg);
    }

    void set_int_persistence(const uint8_t times);

    void set_gain(const Gain gain){
        gainReg = (uint8_t)gain;
        write_reg(RegAddress::Gain, gainReg);
    }

    uint8_t get_id(){
        read_reg(RegAddress::DeviceId, deviceIdReg);
        return deviceIdReg;
    }

    bool is_idle(){
        read_reg(RegAddress::Status, statusReg);
        return statusReg.done_flag;
    }

    void set_power(const bool on){
        enableReg.powerOn = on;
        write_reg(RegAddress::Enable, enableReg);
    }

    void start_conv(){
        enableReg.adcEn = true;
        write_reg(RegAddress::Enable, enableReg);
    }

    void update();

    std::tuple<real_t, real_t, real_t, real_t> get_crgb();

    void init(){
        set_power(true);
        set_integration(240);
        set_gain(Gain::X1);
    }
};

};