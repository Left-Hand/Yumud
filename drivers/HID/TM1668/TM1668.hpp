//这个驱动还未完成
//这个驱动还未测试

//TM1668是天微半导体的一款LED矩阵驱动/按键矩阵扫描芯片


#pragma once

#include "tm1668_prelude.hpp"

namespace ymd::drivers{


class TM1668 final:public TM1668_Prelude{
public:
    using DisplayCommand = TM1668_Transport::DisplayCommand;
    using KeyCode = TM1668_Transport::KeyCode;

    static constexpr auto NAME = "TM1668";

    explicit TM1668(TM1668_Transport && phy):
        transport_(std::move(phy)){;}

    class Display final{
    public:
        Display(TM1668 & owner):
            owner_(owner){;}

        std::span<uint8_t, 4> into_iter(){
            return std::span(buf_);
        }

        void turn_on(){
            display_command_.display_on = true;
        }

        void turn_off(){
            display_command_.display_on = false;
        }


    private:
        DisplayCommand display_command_;
        std::array<uint8_t, 4> buf_;

        TM1668 & owner_;
    };

    class Keyboard final{
    public:
        Keyboard(TM1668 & owner):
            owner_(owner){;}

    private:
        TM1668 & owner_;
    };


    IResult<> write_screen(
        const DisplayCommand cmd, 
        const std::span<const uint8_t, 4> pbuf){
        
        return transport_.write_screen(cmd, pbuf);
    }

    Result<KeyCode, Error> read_key(){
        return transport_.read_key();
    }


private:
    TM1668_Transport transport_;
    Display display{*this};
};

}

