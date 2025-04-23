#pragma once

#include "core/utils/Result.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

class TM1637_Phy final{
public:
    enum class Error{

    };

    enum class PulseWidth:uint8_t{
        _1_16 = 0,
        _2_16,
        _4_16,
        _10_16,
        _11_16,
        _12_16,
        _13_16,
        _14_16,
    };

    struct DataCommand{
        const uint8_t __resv1__:1 = 0;
        uint8_t write_or_read:1;
        uint8_t addr_inc_en:1;
        const uint8_t __resv2__:5 = 0b01001;
    };

    static_assert(sizeof(DataCommand) == 1);

    struct AddressCommand{
        uint8_t addr:3;
        const uint8_t __resv2__:5 = 0b11000;
    };

    static_assert(sizeof(AddressCommand) == 1);

    struct DisplayCommand{
        PulseWidth pulse_width:3;
        uint8_t display_on:1;
        const uint8_t __resv2__:4 = 0b1000;
    };

    static_assert(sizeof(DisplayCommand) == 1);

    class KeyEvent{
    public:
        constexpr Option<uint8_t> row() const {return row_;}
        constexpr Option<uint8_t> col() const {return col_;}

        static constexpr KeyEvent from_u8(const uint8_t data){
            //low 3bit must be 111
            if((data & 0b111) != 0b111) return {None, None};

            //no key pressed
            if(data == 0xff) return {None, None};

            const uint8_t key = data >> 3;
            switch(key){
                case 0b11101: return {Some<uint8_t>(0), Some<uint8_t>(0)};
                case 0b01001: return {Some<uint8_t>(0), Some<uint8_t>(1)};
                case 0b10101: return {Some<uint8_t>(0), Some<uint8_t>(2)};
                case 0b00101: return {Some<uint8_t>(0), Some<uint8_t>(3)};
                case 0b11111: return {Some<uint8_t>(1), Some<uint8_t>(0)};
                case 0b01011: return {Some<uint8_t>(1), Some<uint8_t>(1)};
                case 0b10111: return {Some<uint8_t>(1), Some<uint8_t>(2)};
                case 0b00111: return {Some<uint8_t>(1), Some<uint8_t>(3)};

                case 0b11010: return {Some<uint8_t>(2), Some<uint8_t>(0)};
                case 0b01010: return {Some<uint8_t>(2), Some<uint8_t>(1)};
                case 0b10010: return {Some<uint8_t>(2), Some<uint8_t>(2)};
                case 0b00010: return {Some<uint8_t>(2), Some<uint8_t>(3)};
                case 0b11110: return {Some<uint8_t>(1), Some<uint8_t>(0)};
                case 0b01110: return {Some<uint8_t>(1), Some<uint8_t>(1)};
                case 0b10110: return {Some<uint8_t>(1), Some<uint8_t>(2)};
                case 0b00110: return {Some<uint8_t>(1), Some<uint8_t>(3)};
                // default: while(true);
                default: return {None, None};
            }
        }
    private:
        constexpr KeyEvent(Option<uint8_t> row, Option<uint8_t> col): 
            row_(row),
            col_(col)
        {;}

        Option<uint8_t> row_;
        Option<uint8_t> col_;
    };

    Result<void, Error> write_reg(const uint8_t addr, const uint8_t data){
        return Ok();
    }

    Result<void, Error> write_burst(const uint8_t addr, const std::span<const uint8_t> pbuf){
        return Ok();
    }

    Result<void, Error> write_screen(const uint8_t addr, const std::span<const uint8_t, 6> pbuf){
        return Ok();
    }
private:
    
};

class TM1637 final{
public:
private:
    std::array<uint8_t, 6> buf_;
};

class TM1637_SegDisplayer final{
    static constexpr std::array<uint8_t, 22> SEG_TABLE = {
        0xc0,   //0
        0xf9,   //1
        0xa4,   //2
        0xb0,   //3
        0x99,   //4
        0x92,   //5
        0x82,   //6
        0xf8,   //7
        0x80,   //8
        0x90,   //9
        0x88,   //A
        0x83,   //B
        0xc6,   //C
        0xa1,   //D
        0x86,   //E
        0x8e,   //F

        0x8c,   //P
        0xc1,   //U
        0x91,   //Y
        0x7c,   //L
        0x00,   //全亮
        0xff    //熄灭
    };

    static constexpr uint8_t DOT = 0x80;
    static constexpr uint8_t MINUS = 0x40;

    static constexpr uint8_t char_to_seg(const char c){
        switch(c){
            case 0 ... 9: return SEG_TABLE[c];
            case 'A' ... 'F': return SEG_TABLE[c - 'A' + 10];
            case 'P': return SEG_TABLE[16];
            case 'U': return SEG_TABLE[18];
            case 'Y': return SEG_TABLE[19];
            case 'L': return SEG_TABLE[20];
        }
    }

    [[nodiscard]] static constexpr 
    Result<void, void> render_str(const std::span<const uint8_t> context, const StringView str){
        if(str.size() > context.size()) return Err();
        for(uint8_t i = 0; i < context.size(); ++i){
            if(i >= str.size()) break;
            context[i] = char_to_seg(str[i]);
        }
        return Ok();
    }

    [[nodiscard]] static constexpr 
    Result<void, void> render_digit(const std::span<const uint8_t> context, const int num){
        
    }
};


}