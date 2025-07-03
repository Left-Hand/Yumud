#pragma once

#include <tuple>

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"



namespace ymd::drivers{

struct TCS34725_Prelude{
    enum class Gain:uint8_t{
        _1x = 0b00, 
        _4x = 0b01, 
        _16x = 0b10, 
        _60x = 0b11 
    };

    enum class Error_Kind:uint8_t{
        WrongChipId
    };

    FRIEND_DERIVE_DEBUG(Error_Kind)
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

struct TCS34725_Regs:public TCS34725_Prelude{
    struct R8_Enable:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Enable;
        uint8_t powerOn : 1;
        uint8_t adcEn : 1;
        uint8_t __resv1__ :1;
        uint8_t waitEn : 1;
        uint8_t intEn : 1;
        uint8_t __resv2__ :3;
    }DEF_R8(enable_reg)

    struct R8_Integration:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Integration;
        uint8_t data;
    }DEF_R8(integration_reg)

    struct R8_IntPersistence:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::IntPersistence;
        using Reg8::operator=;
        uint8_t __resv__ :4;
        uint8_t apers   :4;
    }DEF_R8(int_persistence_reg)

    struct R8_LongWait:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::LongWait;
        uint8_t __resv1__ :1;
        uint8_t waitLong : 1;
        uint8_t __resv2__ :6;
    }DEF_R8(long_wait_reg)

    struct R8_Gain:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Gain;
        using Reg8::operator=;
        Gain gain        :2;
        uint8_t __resv2__   :6;
    }DEF_R8(gain_reg)

    struct R8_Status:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Status;
        uint8_t done_flag    :1;
        uint8_t __resv1__   :3;
        uint8_t interrupt_flag     :1;
        uint8_t __resv2__   :3;
    }DEF_R8(status_reg)


    struct R8_WaitTime:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::WaitTime;
        uint8_t data;
    }DEF_R8(wait_time_reg)

    struct R16_LowThr:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::LowThr;
        uint16_t data;
    }DEF_R16(low_thr_reg)

    struct R16_HighThr:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::HighThr;
        uint16_t data;
    }DEF_R16(high_thr_reg)

    struct R8_DeviceId:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::DeviceId;

        // 0x44 = TCS34721 and TCS34725
        // 0x4D = TCS34723 and TCS34727

        static constexpr uint8_t KEY = 0x44;
        uint8_t id;
    }DEF_R8(device_id_reg)

    std::array<uint16_t, 4> crgb = {0};
};


class TCS34725 final:public TCS34725_Regs{
public:
    struct Config{
        Milliseconds integration_time = 240ms;
        Gain gain = Gain::_1x;
    };

    TCS34725(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    TCS34725(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    TCS34725(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(i2c, addr){;}

    TCS34725(const TCS34725 &) = delete;
    TCS34725(TCS34725 &&) = delete;
    ~TCS34725() = default;

        
    IResult<> init(const Config & cfg);

    IResult<> validate();

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
            conv_reg_address_repeated(T::ADDRESS), 
            reg.as_val(), LSB);
        res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(
            conv_reg_address_repeated(T::ADDRESS), reg.as_ref(), LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const RegAddress addr, const std::span<uint16_t> pbuf);

    
    static constexpr uint8_t conv_reg_address_norepeat(const RegAddress addr){
        return (std::bit_cast<uint8_t>(addr) | 0x80);
    }

    static constexpr uint8_t conv_reg_address_repeated(const RegAddress addr){
        return conv_reg_address_norepeat(addr) | (1 << 5);
    }

};

};