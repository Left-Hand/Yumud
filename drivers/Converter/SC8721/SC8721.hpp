#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "concept/pwm_channel.hpp"
#include "concept/analog_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct SC8721_Collections{
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b01100000);
    using RegAddress = uint8_t;

    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class DeadZone:uint8_t{
        _20ns,
        _40ns
    };

    enum class SwitchFreq:uint8_t{
        // Switching frequency setting:
        // 00: 260kHz 
        // 01: 500kHz(default)
        // 10: 720kHz
        // 11: 920kHz
        _260kHz = 0b00,
        _500kHz = 0b01,
        _720kHz = 0b10,
        _920kHz = 0b11
    };

    enum class SlopComp:uint8_t{
        _0,
        _50,
        _100,
        _150
    };

    struct Status{
        uint8_t short_circuit:1;
        uint8_t vout_vin_h:1;
        uint8_t thermal_shutdown:1;
        uint8_t ocp:1;
        uint8_t vin_ovp:1;
        uint8_t on_cv:1;
        uint8_t on_cc:1;
    };

    static_assert(sizeof(Status)==1,"sizeof(Status)==1");

};

struct SC8721_Regs:public SC8721_Collections{
    struct R8_CSO:public Reg8<>{
        scexpr RegAddress address = 0x01;

        uint8_t cso_set;
    }DEF_R8(cso_reg)

    struct R8_SlopeComp:public Reg8<>{
        scexpr RegAddress address = 0x02;

        uint8_t slop_comp:1;
        uint8_t :7;
    }DEF_R8(slope_comp_reg)

    struct R8_VoutSetMsb:public Reg8<>{//msb
        scexpr RegAddress address = 0x03;

        uint8_t vout_set_msb;
    }DEF_R8(vout_set_msb_reg)

    struct R8_VoutSetLsb:public Reg8<>{//msb
        scexpr RegAddress address = 0x04;

        uint8_t vout_set_lsb:2;
        uint8_t fb_dir:1;
        uint8_t fb_on:1;
        uint8_t fb_sel:1;
        uint8_t :3;
    }DEF_R8(vout_set_lsb_reg)

    struct R8_GlobalCtrl:public Reg8<>{//msb
        scexpr RegAddress address = 0x05;

        uint8_t :1;
        uint8_t reg_load:1;
        uint8_t dis_dcdc:1;
        uint8_t :5;
    }DEF_R8(global_ctrl_reg)

    struct R8_SysSet:public Reg8<>{//msb
        scexpr RegAddress address = 0x06;

        uint8_t :4;
        uint8_t en_vinreg:1;
        uint8_t :1;
        uint8_t ext_dt:1;
        uint8_t en_pwm:1;
    }DEF_R8(sys_set_reg)

    struct R8_FreqSet:public Reg8<>{
        scexpr RegAddress address = 0x08;

        uint8_t freq_set:2;
        uint8_t :6;
    }DEF_R8(freq_set_reg)

    struct R8_Status1:public Reg8<>{
        scexpr RegAddress address = 0x09;

        uint8_t ocp:1;
        uint8_t :2;
        uint8_t thd:1;
        uint8_t :2;
        uint8_t vout_vin_h:1;
        uint8_t vout_short:1;
    }DEF_R8(status1_reg)

    struct R8_Status2:public Reg8<>{
        scexpr RegAddress address = 0x0A;

        uint8_t :1;
        uint8_t ibus_flag:1;
        uint8_t vinreg_flag:1;
        uint8_t :4;
        uint8_t vinovp:1;
    }DEF_R8(status2_reg)
};

class SC8721 final:public SC8721_Regs{
public:
    SC8721(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    SC8721(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    SC8721(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> set_target_voltage(const real_t volt);

    [[nodiscard]] IResult<> enable_external_fb(const Enable en = EN);

    [[nodiscard]] IResult<> set_dead_zone(const DeadZone dz);

    [[nodiscard]] IResult<> set_switch_freq(const SwitchFreq freq);

    [[nodiscard]] IResult<Status> get_status();

    [[nodiscard]] IResult<> set_slope_comp(const SlopComp sc);
private:
    hal::I2cDrv i2c_drv_;

    [[nodiscard]] IResult<> write_reg(const RegAddress address, const uint8_t reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const RegAddress address, uint8_t & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const RegAddress addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = write_reg(reg.address, reg.as_val());
        if(res.is_ok()) reg.apply();
        return res;
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(reg.address, reg.as_ref());
    }
};

}