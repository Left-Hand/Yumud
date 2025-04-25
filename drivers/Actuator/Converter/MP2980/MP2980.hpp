#pragma once

#include "core/io/regs.hpp"
#include "concept/pwm_channel.hpp"
#include "concept/analog_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{

class MP2980{
public:
    enum class VrefSlewRate:uint8_t{
        _38_V_S,
        _50_V_S,
        _75_V_S,
        _150_V_S
    };

    enum class OvpMode:uint8_t{
        NoProtect,
        Discharge, // default
        LatchOff
    };

    enum class OcpMode:uint8_t{
        CycleByCycle,
        Hiccup, // default
        LatchOff
    };

    enum class BuckBoostFsw:uint8_t{
        _40_P,
        _80_P
    };

    enum class Fsw:uint8_t{
        _200_kHz,
        _300_kHz,
        _400_kHz,
        _600_kHz
    };

    enum class CurrLimitThreshold:uint8_t{
        _27_9_mV,
        _33_3_mV,
        _39_3_mV,
        _45_1_mV,
        _51_2_mV,
        _56_8_mV,
        _62_8_mV,
        _68_7_mV,
    };
protected:
    using RegAddress = uint8_t;
    uint fb_up_res_ohms = 90.9 * 1000;
    uint fb_down_res_ohms = 10 * 1000;

    hal::I2cDrv i2c_drv_;

    struct RefReg:public Reg16<>{
        scexpr RegAddress address = 0x00;

        uint16_t :16;

        RefReg & set(const uint16_t data){
            auto & self = *this;
            self.as_bytes()[0] = std::byte(data & 0b111);
            self.as_bytes()[1] = std::byte(data >> 3);

            return *this;
        }

        uint16_t get() const {
            auto & self = *this;
            return (uint8_t(self.as_bytes()[1]) << 3) | uint8_t(self.as_bytes()[0]);
        }
    };

    struct Ctrl1Reg:public Reg8<>{
        scexpr RegAddress address = 0x02;

        uint8_t en_pwr:1;
        uint8_t go_bit:1;
        uint8_t :1;
        uint8_t png_latch:1;
        uint8_t dither:1;
        uint8_t dischg:1;
        uint8_t sr:2;
    };

    struct Ctrl2Reg:public Reg8<>{
        scexpr RegAddress address = 0x03;

        uint8_t ovp_mode:1;
        uint8_t ocp_mode:1;
        uint8_t bb_fsw:1;
        uint8_t :1;
        uint8_t fsw:2;
    };

    struct IlimReg:public Reg8<>{
        scexpr RegAddress address = 0x04;

        uint8_t ilim:3;
        uint8_t :5;
    };

    struct Interrupts{
        uint8_t png:1;
        uint8_t ocp:1;
        uint8_t ovp:1;
        uint8_t :1;
        uint8_t otp:1;
        uint8_t :3;
    };

    struct StatusReg:public Reg8<>, public Interrupts{
        scexpr RegAddress address = 0x05;
    };

    struct MaskReg:public Reg8<>, public Interrupts{
        using Reg8::operator =;
        scexpr RegAddress address = 0x06;
    };

    RefReg ref_reg = {};
    Ctrl1Reg ctrl1_reg = {};
    Ctrl2Reg ctrl2_reg = {};
    IlimReg ilim_reg = {};
    MaskReg mask_reg = {};
    StatusReg status_reg = {};

    hal::BusError write_reg(const RegAddress address, const uint8_t reg){
        return i2c_drv_.write_reg(uint8_t(address), reg);
    }

    hal::BusError read_reg(const RegAddress address, uint8_t & reg){
        return i2c_drv_.read_reg(uint8_t(address), reg);
    }

    hal::BusError write_reg(const RegAddress address, const uint16_t reg){
        return i2c_drv_.write_reg(uint8_t(address), reg, LSB);
    }

    hal::BusError read_reg(const RegAddress address, uint16_t & reg){
        return i2c_drv_.read_reg(uint8_t(address), reg, LSB);
    }

public:

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b01100000);

    MP2980(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    MP2980(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    MP2980(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    MP2980 & setFeedBackVref(const real_t vref);
    MP2980 & setFeedBackVrefMv(const uint vref_mv);
    MP2980 & enablePowerSwitching(const bool en = true);
    MP2980 & enableVrefChangeFunc(const bool en = true);
    MP2980 & setPngState(const bool state);
    MP2980 & enableDither(const bool en = true);
    MP2980 & setVrefSlewRate(const VrefSlewRate slewrate);
    MP2980 & setOvpMode(const OvpMode mode);
    MP2980 & setOcpMode(const OcpMode mode);
    MP2980 & setFsw(const Fsw fsw);
    MP2980 & setBuckBoostFsw(const BuckBoostFsw fsw);
    MP2980 & setCurrLimitThreshold(const CurrLimitThreshold threshold);
    Interrupts interrupts();
    MP2980 & setInterruptMask(const Interrupts mask);
    MP2980 & setOutputVolt(const real_t output_volt);
    MP2980 & init();
};

}