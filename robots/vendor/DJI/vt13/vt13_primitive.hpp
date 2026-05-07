#pragma once

#include <cstdint>
#include <array>
#include <span>


namespace ymd::robots::dji::vt13{

namespace utils{
static constexpr float parse_u11(const uint16_t bits){
    return static_cast<float>(bits - 1024) / 660.0f;
}


}


#define VT13_KEY_SPECIFIER_BIT(KEY_NAME, BIT_INDEX)                     \
    template <typename Self>                                             \
    [[nodiscard]] constexpr auto KEY_NAME(this Self&& self) {            \
        return make_bitfield_proxy<BIT_INDEX, BIT_INDEX + 1, bool>(self.bits);       \
    }

struct [[nodiscard]] KeySpecifiers final {
    uint16_t bits;

    VT13_KEY_SPECIFIER_BIT(key_w,     0)
    VT13_KEY_SPECIFIER_BIT(key_a,     1)
    VT13_KEY_SPECIFIER_BIT(key_s,     2)
    VT13_KEY_SPECIFIER_BIT(key_d,     3)
    VT13_KEY_SPECIFIER_BIT(key_shift, 4)
    VT13_KEY_SPECIFIER_BIT(key_ctrl,  5)

    VT13_KEY_SPECIFIER_BIT(key_q,  6)
    VT13_KEY_SPECIFIER_BIT(key_e,  7)
    VT13_KEY_SPECIFIER_BIT(key_r,  8)
    VT13_KEY_SPECIFIER_BIT(key_f,  9)
    VT13_KEY_SPECIFIER_BIT(key_g,  10)
    VT13_KEY_SPECIFIER_BIT(key_z,  11)
    VT13_KEY_SPECIFIER_BIT(key_x,  12)
    VT13_KEY_SPECIFIER_BIT(key_c,  13)
    VT13_KEY_SPECIFIER_BIT(key_v,  14)
    VT13_KEY_SPECIFIER_BIT(key_b,  15)
};

#undef VT13_KEY_SPECIFIER_BIT



struct __attribute__((packed)) [[nodiscard]]Packet final{
    uint16_t right_horizen : 11; 
    uint16_t right_vertical : 11; 
    uint16_t left_veritcal : 11; 
    uint16_t left_horizen : 11;  
    uint8_t mode_sw : 2;
    uint8_t pause_pressed : 1; 
    uint8_t fn1_pressed : 1;
    uint8_t fn2_pressed : 1;
    uint16_t wheel_pressed : 11;
    uint8_t trigger_pressed : 1; 

    uint8_t __resv__ : 3;

    int16_t mouse_x : 16;     
    int16_t mouse_y : 16;     
    int16_t mouse_z : 16;     
    uint8_t mouse_left : 2;   
    uint8_t mouse_right : 2;  
    uint8_t mouse_middle : 2; 

    uint8_t __resv2__ : 2;

    KeySpecifiers keys;
};

static_assert(sizeof(Packet) == 17);
}