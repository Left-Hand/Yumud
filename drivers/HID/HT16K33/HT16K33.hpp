#pragma once

//这个驱动还未完成

//HT16K33是一款真I2C的Led矩阵扫描/按键扫描芯片 同时带有中断引脚

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct HT16K33_Collections{
    //  1 1 1 0 A2 A1 A0 0
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(
        0b1110000);

    enum class Package:uint8_t{
        SOP20,
        SOP24,
        SOP28
    };

    enum class PulseDuty{
        _1_16, _2_16, _3_16, _4_16,
        _5_16, _6_16, _7_16, _8_16,
        _9_16, _10_16, _11_16, _12_16,
        _13_16, _14_16, _15_16, _16_16
    };

    enum class Error_Kind{
        DisplayBitIndexOutOfRange,
        DisplayByteIndexOutOfRange
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddress = uint8_t;

    enum class BlinkFreq:uint8_t{
        OFF = 0b00,
        _2HZ = 0b01,
        _1HZ = 0b01,
        _0_5HZ = 0b01,
    };

    enum class IntPinFunc:uint8_t{
        AsRowDriver,
        AsExtiActiveLow,
        AsExtiActiveHigh
    };

    struct Command{

    public:
        template<typename T>
        requires (sizeof(T) == 1)
        constexpr Command(const T cmd):raw_(std::bit_cast<uint8_t>(cmd)){;}

        uint8_t as_u8() const{
            return raw_;
        }
    private:
        uint8_t raw_;
    };
};

class HT16K33_Phy final:public HT16K33_Collections{
public:
    HT16K33_Phy(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    [[nodiscard]] IResult<> write_command(const Command cmd){
        if(const auto res = i2c_drv_.write_blocks<>(cmd.as_u8(), LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> write_data(const RegAddress addr, const uint8_t data){
        if(const auto res = i2c_drv_.write_reg(addr, data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> write_burst(
        const RegAddress addr, 
        const std::span<const uint8_t> pdata
    ){
        // page 23
        // After reaching the display memory location 0X0FH the pointer will reset to 0X00H (display	
        // memory).
        if(const auto res = i2c_drv_.write_burst(addr, pdata);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_data(const RegAddress addr, const uint8_t data){
        if(const auto res = i2c_drv_.write_reg(addr, data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(
        const RegAddress addr, 
        const std::span<uint8_t> pdata
    ){
        if(const auto res = i2c_drv_.read_burst(addr, pdata);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


private:
    hal::I2cDrv i2c_drv_;
};

struct HT16K33_Regs:public HT16K33_Collections{
    struct SetDataPtr:public Reg8<>{
        uint8_t addr:4;
        const uint8_t __resv__:4 = 0b0000; 
    };
    CHECK_R8(SetDataPtr)

    struct SystemSetup:public Reg8<>{
        uint8_t turn_on:1;
        uint8_t __resv__:7 = 0b0010000;

        SystemSetup(const bool _turn_on):
            turn_on(bool(_turn_on))
        {;}
    };

    CHECK_R8(SystemSetup)

    struct KeyDataPtr:public Reg8<>{
        uint8_t key:3;
        const uint8_t __resv__:5 = 0b01000;
    };

    CHECK_R8(KeyDataPtr)

    struct IntFlagPtr:public Reg8<>{
        const uint8_t __resv__ = 0b01100000;
    };

    CHECK_R8(IntFlagPtr)

    struct DisplaySetup:public Reg8<>{
        uint8_t display_on:1;
        BlinkFreq blink_freq:2;
        uint8_t __resv__:5 = 0b10000;
    };

    CHECK_R8(DisplaySetup)

    struct IntSet:public Reg8<>{
        IntPinFunc int_pin_func:2;
        const uint8_t __resv__:6 = 0b101000;

        IntSet(const IntPinFunc _func):
            int_pin_func(_func)
        {;}
    };

    CHECK_R8(IntSet)

    struct DimmingSet:public Reg8<>{
        uint8_t dimming:4;
        const uint8_t __resv__:4 = 0b1000;
    };

    CHECK_R8(DimmingSet)

    //  HOLTEK use only
    // struct TestMode:public Reg8<>{
    //     const uint8_t __resv__ = 0b1111111;
    // };
    static constexpr size_t GC_RAM_SIZE = 16;
    static constexpr size_t KEY_RAM_SIZE = 3;
    using GcRam = std::array<uint8_t, GC_RAM_SIZE>;
    using KeyRam = std::array<uint8_t, KEY_RAM_SIZE>;
    
    GcRam gc_ram_;
};

class HT16K33 final:public HT16K33_Regs{
public:
    IResult<> validate();

    IResult<> init();
private:
    using Phy = HT16K33_Phy;
    Phy phy_;

    IResult<> commit_gcram_to_displayer(){
        // return phy_.write_burst(SetDataPtr{}, gc_ram_);
        TODO();
    }

    IResult<> set_display_bit(const size_t num, const bool value){
        static constexpr size_t VALUE_WIDTH = magic::type_to_bits_v<GcRam::value_type>;
        static constexpr size_t MAX_NUM = VALUE_WIDTH * GC_RAM_SIZE;
        if(num >= MAX_NUM) return Err(Error::DisplayBitIndexOutOfRange);

        const auto i = num / VALUE_WIDTH;
        const auto mask = 1 << (num % VALUE_WIDTH);
        if(value) gc_ram_[i] |= mask;
        else gc_ram_[i] &= ~mask;
    }

    IResult<> set_display_byte(const size_t index, const uint8_t value){
        if(index >= GC_RAM_SIZE) return Err(Error::DisplayByteIndexOutOfRange);
        gc_ram_[index] = value;
        return Ok();
    }

    IResult<> clear_display_content(){
        std::fill(gc_ram_.begin(), gc_ram_.end(), 0);
        return commit_gcram_to_displayer();
    }

    IResult<> write_command(const Command cmd){
        return phy_.write_command(cmd);
    }

    IResult<> system_setup(const Enable en){
        return write_command(SystemSetup{en == EN});
    }

    IResult<> set_int_pin_func(const IntPinFunc func){
        return write_command(IntSet{func});
    }
};
}