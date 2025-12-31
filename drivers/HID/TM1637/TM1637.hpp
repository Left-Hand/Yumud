//这个驱动已经完成
//这个驱动正在测试
    //显示已经完成测试 需要进一步添加业务语法糖
    //按键扫描还未测试 目前尚未有硬件平台

//TM1637是天微半导体的一款LED矩阵驱动/按键矩阵扫描芯片


#pragma once

#include "tm1637_prelude.hpp"

namespace ymd::drivers{

//TM1637本体
class TM1637 final:public _TM1637_Prelude{
public:
    using Phy = TM1637_Transport;

    TM1637(hal::Gpio & scl_pin, hal::Gpio & sda_pin): 
        transport_(TM1637_Transport(scl_pin, sda_pin)){;}


    [[nodiscard]]
    IResult<> flush();
    
    [[nodiscard]]
    IResult<Option<KeyPlacement>> read_key();
    [[nodiscard]]
    IResult<> set(const size_t pos, const uint8_t val){
        if(pos > CGRAM_MAX_LEN) return Err(Error::IndexOutOfRange);
        buf_.set(pos, val);
        return Ok();
    }


    [[nodiscard]]
    IResult<> set_display_dutycycle(const real_t dutycycle);
    
private:
    using Buf = DisplayBuf<uint8_t, CGRAM_MAX_LEN>;

    Phy transport_;
    Buf buf_;
    DisplayCommand disp_cmd_;
    bool is_on_display_else_readkey_ = true;

    [[nodiscard]] IResult<> switch_to_display();
    [[nodiscard]] IResult<> switch_to_readkey();


};

//段显示器
struct SegDisplayer final{
    static constexpr std::array<uint8_t, 22> SEG_TABLE = {
        uint8_t(~0xc0),   //0
        uint8_t(~0xf9),   //1
        uint8_t(~0xa4),   //2
        uint8_t(~0xb0),   //3
        uint8_t(~0x99),   //4
        uint8_t(~0x92),   //5
        uint8_t(~0x82),   //6
        uint8_t(~0xf8),   //7
        uint8_t(~0x80),   //8
        uint8_t(~0x90),   //9
        uint8_t(~0x88),   //A
        uint8_t(~0x83),   //B
        uint8_t(~0xc6),   //C
        uint8_t(~0xa1),   //D
        uint8_t(~0x86),   //E
        uint8_t(~0x8e),   //F

        uint8_t(~0x8c),   //P
        uint8_t(~0xc1),   //U
        uint8_t(~0x91),   //Y
        uint8_t(~0x7c),   //L
        uint8_t(~0x00),   //全亮
        uint8_t(~0xff)    //熄灭
    };

    // static constexpr uint8_t SEG_MINUS = 0x4f;
    static constexpr uint8_t SEG_MINUS = 0xC0;

    static constexpr uint8_t DOT = 0x80;
    static constexpr uint8_t MINUS = 0x40;

    static constexpr uint8_t char_to_seg(const char c){
        switch(c){
            case '0' ... '9': return uint8_t(SEG_TABLE[c - '0']);
            case 'A' ... 'F': return uint8_t(SEG_TABLE[c - 'A' + 10]);
            case 'a' ... 'f': return uint8_t(SEG_TABLE[c - 'a' + 10]);
            case 'P': return uint8_t(SEG_TABLE[16]);
            case 'U': return uint8_t(SEG_TABLE[17]);
            case 'Y': return uint8_t(SEG_TABLE[18]);
            case 'L': return uint8_t(SEG_TABLE[19]);
            default: sys::abort();
        }
    }

    static constexpr uint8_t digit_to_seg(const uint8_t digit){
        return char_to_seg((digit % 10) + '0');
    }

    [[nodiscard]] static constexpr 
    Result<void, void> render_str(const std::span<uint8_t> context, const StringView str){
        if(str.size() > context.size()) return Err();
        for(uint8_t i = 0; i < context.size(); ++i){
            if(i >= str.size()) break;
            context[i] = char_to_seg(str[i]);
        }
        return Ok();
    }

    [[nodiscard]] static constexpr 
    Result<void, void> render_digit(const std::span<uint8_t> context, const int num){
        const auto len = context.size();
        if(len >= 2){
            context[0] = digit_to_seg(num % 10);
            context[1] = digit_to_seg(num / 10);

        }
        return Ok();
    }
};


}