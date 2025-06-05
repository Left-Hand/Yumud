#pragma once

//这个驱动还未完成

//HT16K33是一款真I2C的Led矩阵扫描/按键扫描芯片 同时带有中断引脚

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "drivers/HID/Event.hpp"


namespace ymd::drivers{

struct HT16K33_Collections{

    //  1 1 1 0 A2 A1 A0 0
    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u7(0b1110000);

    static constexpr auto make_address_sop28(
        const BoolLevel A2,
        const BoolLevel A1,
        const BoolLevel A0
    ){
        return hal::I2cSlaveAddr<7>::from_u7(0b1110000
            | ((A2 == HIGH) ? 0b100 : 0)
            | ((A1 == HIGH) ? 0b010 : 0)
            | ((A0 == HIGH) ? 0b001 : 0)
        );
    }

    static constexpr auto make_address_sop24(
        const BoolLevel A1,
        const BoolLevel A0
    ){
        return make_address_sop28(
            LOW, A1, A0
        );
    }

    enum class Package:uint8_t{
        SOP20,
        SOP24,
        SOP28
    };

    enum class PulseDuty:uint8_t{
        _1_16, _2_16, _3_16, _4_16,
        _5_16, _6_16, _7_16, _8_16,
        _9_16, _10_16, _11_16, _12_16,
        _13_16, _14_16, _15_16, _16_16
    };

    enum class BlinkFreq:uint8_t{
        OFF = 0b00,
        _2HZ = 0b01,
        _1HZ = 0b01,
        _0_5HZ = 0b01,
    };

    enum class IntPinFunc:uint8_t{
        AsRowDriver,
        InterruptActiveLow,
        InterruptActiveHigh
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


    enum class Error_Kind:uint8_t{
        DisplayBitIndexOutOfRange,
        DisplayByteIndexOutOfRange,
        KeyColumnOutOfRange,
        KeyRowOutOfRange
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddress = uint8_t;

    struct Settings{
        struct SOP28Settings{
            static constexpr Package PACKAGE = Package::SOP28;
        };

        struct SOP20Settings{
            static constexpr Package PACKAGE = Package::SOP20;
        };
    };
};



struct HT16K33_Regs:public HT16K33_Collections{
    struct SetDataPtrCommand:public Reg8<>{
        uint8_t addr:4;
        const uint8_t __resv__:4 = 0b0000; 
    };
    CHECK_R8(SetDataPtrCommand)

    struct SystemSetupCommand:public Reg8<>{
        uint8_t turn_on:1;
        uint8_t __resv__:7 = 0b0010000;

        SystemSetupCommand(const bool _turn_on):
            turn_on(_turn_on){;}
    };

    CHECK_R8(SystemSetupCommand)

    struct KeyDataPtrCommand:public Reg8<>{
        uint8_t key:3;
        const uint8_t __resv__:5 = 0b01000;
    };

    CHECK_R8(KeyDataPtrCommand)

    struct IntFlagPtr:public Reg8<>{
        const uint8_t __resv__ = 0b01100000;
    };

    CHECK_R8(IntFlagPtr)

    struct DisplaySetupCommand:public Reg8<>{
        uint8_t display_on:1;
        BlinkFreq blink_freq:2;
        uint8_t __resv__:5 = 0b10000;
    };

    CHECK_R8(DisplaySetupCommand)

    struct IntSet:public Reg8<>{
        IntPinFunc int_pin_func:2;
        const uint8_t __resv__:6 = 0b101000;

        IntSet(const IntPinFunc _func):
            int_pin_func(_func)
        {;}
    };

    CHECK_R8(IntSet)

    struct DimmingSet:public Reg8<>{
        PulseDuty dimming:4;
        const uint8_t __resv__:4 = 0b1110;
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

class HT16K33_Phy final:public HT16K33_Collections{
public:
    HT16K33_Phy(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    HT16K33_Phy(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

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
        const std::span<const uint8_t> pbuf
    ){
        // page 23
        // After reaching the display memory location 0X0FH the pointer will reset to 0X00H (display	
        // memory).
        if(const auto res = i2c_drv_.write_burst(addr, pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_data(const RegAddress addr, uint8_t & data){
        if(const auto res = i2c_drv_.read_reg(addr, data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(
        const RegAddress addr, 
        const std::span<uint8_t> pbuf
    ){
        if(const auto res = i2c_drv_.read_burst(addr, pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


private:
    hal::I2cDrv i2c_drv_;
};

class HT16K33 final:public HT16K33_Regs{
public:
    template<typename Set>
    HT16K33(Set && set, const hal::I2cDrv & i2c_drv):
        phy_(i2c_drv)
    {
        resetting(std::forward<Set>(set));
    }
    
    template<typename Set>
    void resetting(Set && set){
        package_ = set.PACKAGE;
    }

    IResult<> init();
    IResult<> validate();


    IResult<> set_int_pin_func(const IntPinFunc func);

    IResult<std::bitset<8>> get_int_status();


    struct KeyData{
        constexpr bool test(const uint8_t x ,const uint8_t y) const {
            const bool is_high_byte = x >= 8;
            const auto byte = buf_[y * 2 + is_high_byte];
            return byte & (1 << (x % 8));
        }

        template<size_t R>
        requires (R < 3)
        constexpr std::bitset<13> row_as_bitset() const {
            const auto low_byte = buf_[R * 2];
            const auto high_byte = buf_[R * 2 + 1];
            return std::bitset<13>((high_byte << 8) | low_byte);
        }

        constexpr std::span<uint8_t> as_bytes(){
            return std::span(buf_);
        }

        constexpr std::span<const uint8_t> as_bytes() const {
            return std::span(buf_);
        }

        friend OutputStream & operator <<(OutputStream & os, const KeyData & self){
            const auto b3 = std::to_array<std::bitset<13>>({
                self.row_as_bitset<0>(),
                self.row_as_bitset<1>(),
                self.row_as_bitset<2>()
            });

            return os << b3;
        }
    private:
        std::array<uint8_t, 6> buf_;
    };

    IResult<KeyData> get_key_data();
private:
    using Phy = HT16K33_Phy;
    Phy phy_;
    Package package_;

    IResult<> commit_gcram_to_displayer();

    IResult<> set_display_bit(const size_t num, const bool value);

    IResult<> set_display_byte(const size_t index, const uint8_t value);

    IResult<> clear_display_content();

    IResult<> write_command(const Command cmd);

    IResult<> enable_system_setup(const Enable en);

};
}