#pragma once

#include <tuple>

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"



namespace ymd::drivers{

struct TCS34725_Collections{
    enum class Gain:uint8_t{
        X1 = 0, X4, X16, X60 
    };

    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
    

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

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x52);
};

struct TCS34725_Regs:public TCS34725_Collections{
    struct EnableReg:public Reg8<>{
        static constexpr auto address = RegAddress::Enable;
        uint8_t powerOn : 1;
        uint8_t adcEn : 1;
        uint8_t __resv1__ :2;
        uint8_t waitEn : 1;
        uint8_t intEn : 1;
        uint8_t __resv2__ :3;
    };

    struct IntPersistenceReg:public Reg8<>{
        static constexpr auto address = RegAddress::IntPersistence;
        using Reg8::operator=;
        uint8_t __resv__ :4;
        uint8_t apers   :4;
    };

    struct LongWaitReg:public Reg8<>{
        static constexpr auto address = RegAddress::LongWait;
        uint8_t __resv1__ :1;
        uint8_t waitLong : 1;
        uint8_t __resv2__ :6;
    };

    struct GainReg:public Reg8<>{
        static constexpr auto address = RegAddress::Gain;
        using Reg8::operator=;
        Gain gain        :2;
        uint8_t __resv2__   :6;
    };

    struct StatusReg:public Reg8<>{
        static constexpr auto address = RegAddress::Status;
        uint8_t done_flag    :1;
        uint8_t __resv1__   :3;
        uint8_t interrupt_flag     :1;
        uint8_t __resv2__   :3;
    };

    struct IntehrationReg:public Reg8<>{
        static constexpr auto address = RegAddress::Integration;
        uint8_t data;
    };

    struct WaitTimeReg:public Reg8<>{
        static constexpr auto address = RegAddress::WaitTime;
        uint8_t data;
    };

    struct LowThrReg:public Reg16<>{
        static constexpr auto address = RegAddress::LowThr;
        uint16_t data;
    };

    struct HighThrReg:public Reg16<>{
        static constexpr auto address = RegAddress::HighThr;
        uint16_t data;
    };

    struct DeviceIdReg:public Reg8<>{
        static constexpr auto address = RegAddress::DeviceId;
        uint8_t id;
    };

    EnableReg enable_reg = {};
    IntehrationReg integration_reg = {};
    WaitTimeReg wait_time_reg = {};
    LowThrReg low_thr_reg = {};
    HighThrReg high_thr_reg = {};
    IntPersistenceReg int_persistence_reg = {};
    LongWaitReg long_wait_reg = {};
    GainReg gain_reg = {};
    DeviceIdReg device_id_reg = {};
    StatusReg status_reg = {};
    std::array<uint16_t, 4> crgb = {0};
};


class TCS34725:public TCS34725_Regs{
public:
    TCS34725(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    TCS34725(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    TCS34725(
        hal::I2c & bus, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(bus, addr){;}


        
    IResult<> init();
    IResult<> set_integration_time(const Milliseconds ms);

    IResult<> set_wait_time(const Milliseconds ms);

    IResult<> set_int_thr_low(const uint16_t thr);

    IResult<> set_int_thr_high(const uint16_t thr);

    IResult<> set_int_persistence(const uint8_t times);

    IResult<> set_gain(const Gain gain);

    IResult<uint8_t> get_id();
    IResult<bool> is_idle();

    IResult<> set_power(const bool on);
    IResult<> start_conv();
    IResult<> update();

    std::tuple<real_t, real_t, real_t, real_t> get_crgb();

private:
    hal::I2cDrv i2c_drv_;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(
            conv_reg_address(reg.address), 
            reg.as_val(), LSB);
        res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(
            conv_reg_address(reg.address), reg.as_ref(), LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const RegAddress addr, const std::span<uint16_t> pdata);

    
    static constexpr uint8_t conv_reg_address(const RegAddress addr, bool repeat = true){
        return ((uint8_t) addr) | 0x80 | (repeat ? 1 << 5 : 0);
    }

};

};