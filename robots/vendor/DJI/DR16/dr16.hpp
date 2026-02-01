#pragma once

#include "hal/bus/uart/hw_singleton.hpp"


namespace ymd::robots::dji::dr16{

#pragma pack(push, 1)
struct FrameContext{
    using Self =  FrameContext;

    uint16_t ch0:11; //[11 : 0]
    uint16_t ch1:11; //[22 : 11]
    uint16_t ch2:11; //[33 : 22]
    uint16_t ch3:11; //[44 : 33]
    uint16_t s1:2;  //[46 : 44]
    uint16_t s2:2;  //[48 : 46]

    uint16_t mouse_x:16; //[64 : 48]
    uint16_t mouse_y:16; //[80 : 64]
    uint16_t mouse_z:16; //[96 : 80]
    uint16_t mouse_l:8; //[104 : 96]
    uint16_t mouse_r:8; //[112 : 104]
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

    [[nodiscard]] std::span<uint8_t> as_bytes_mut() {
        return {reinterpret_cast<uint8_t*>(this), sizeof(Self)};
    }

    void reset(){
        std::fill(as_bytes_mut().begin(), as_bytes_mut().end(), 0x00);
    }
};
#pragma pack(pop)

static_assert(sizeof(FrameContext) == 18);


struct Dr16_ParseReceiver{

};

};