#pragma once

#include "hal/bus/uart/uart.hpp"
#include "hal/analog/adc/adc_channel.hpp"


namespace ymd::robots{

struct DR16_Prelude{
    struct Frame{
        using Self =  Frame;

        uint16_t ch0:11;
        uint16_t ch1:11;
        uint16_t ch2:11;
        uint16_t ch3:11;
        uint16_t s1:2;
        uint16_t s2:2;
        uint16_t mouse_x:16;
        uint16_t mouse_y:16;
        uint16_t mouse_z:16;
        uint16_t mouse_l:8;
        uint16_t mouse_r:8;
        union{
            struct{
                uint16_t key_w:1;      
                uint16_t key_a:1;      
                uint16_t key_s:1;      
                uint16_t key_d:1;      
                uint16_t key_q:1;      
                uint16_t key_e:1;      
                uint16_t key_shift:1;      
                uint16_t key_ctrl:1;      
            };
            uint16_t keys:16;
        }__packed;
        uint16_t __resv__:16;

        [[nodiscard]] std::span<const uint8_t> as_bytes() const{
            return {reinterpret_cast<const uint8_t*>(this), sizeof(Self)};
        }

        [[nodiscard]] std::span<uint8_t> as_mut_bytes() {
            return {reinterpret_cast<uint8_t*>(this), sizeof(Self)};
        }

        void reset(){
            std::fill(as_mut_bytes().begin(), as_mut_bytes().end(), 0x00);
        }
    }__packed;

    static_assert(sizeof(Frame) == 18);
};

class DR16:public DR16_Prelude{
public:
    using DBUS = hal::Uart;
    explicit DR16(DBUS & inst):inst_(inst){}
private:
    DBUS & inst_;
};

};