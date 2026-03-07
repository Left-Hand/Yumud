//这个驱动还未完成
//这个驱动还未测试

//TM1650是天微半导体的一款LED矩阵驱动/按键矩阵扫描芯片


#pragma once

#include "tm1650_prelude.hpp"


namespace ymd::drivers{


#if 0

class TM1650_Display final{
public:
    explicit TM1650_Display(TM1650 & owner):
        owner_(owner){;}

    std::span<uint8_t, 4> into_iter(){
        return std::span(buf_);
    }

    void set_brightness(const uint8_t brightness){
        display_command_.lim = brightness;
    }

    void turn_on(){
        display_command_.display_on = true;
    }

    void turn_off(){
        display_command_.display_on = false;
    }

    void enable_seg7(const Enable en){
        display_command_.seg7_else_sge8 = (en == EN);
    }

private:
    DisplayCommand display_command_;
    std::array<uint8_t, 4> buf_;

    TM1650 & owner_;
};

class TM1650_Keyboard final{
public:
    explicit TM1650_Keyboard(TM1650 & owner):
        owner_(owner){;}

private:
    TM1650 & owner_;
};

#endif
}