#pragma once

#include <span>
#include "core/utils/bits/bitfield_proxy.hpp"


namespace ymd::robots::dji::dr16{


#define DR16_KEY_SPECIFIER_BIT(KEY_NAME, BIT_INDEX)                     \
    template <typename Self>                                             \
    [[nodiscard]] constexpr auto KEY_NAME(this Self&& self) {            \
        return make_bitfield_proxy<BIT_INDEX, BIT_INDEX + 1, bool>(self.bits);       \
    }

struct alignas(2) [[nodiscard]] KeySpecifiers final {
    uint16_t bits;

    DR16_KEY_SPECIFIER_BIT(key_w,     0)
    DR16_KEY_SPECIFIER_BIT(key_a,     1)
    DR16_KEY_SPECIFIER_BIT(key_s,     2)
    DR16_KEY_SPECIFIER_BIT(key_d,     3)
    DR16_KEY_SPECIFIER_BIT(key_q,     4)
    DR16_KEY_SPECIFIER_BIT(key_e,     5)
    DR16_KEY_SPECIFIER_BIT(key_shift, 6)
    DR16_KEY_SPECIFIER_BIT(key_ctrl,  7)
};

struct alignas(2) [[nodiscard]] MouseAnalogCode final{
    int16_t bits;
};

#undef DR16_KEY_SPECIFIER_BIT


#pragma pack(push, 1)
struct [[nodiscard]] alignas(2) FrameContext final{
    using Self =  FrameContext;



    uint16_t ch0:11;
    uint16_t ch1:11;
    uint16_t ch2:11;
    uint16_t ch3:11;
    uint16_t s1:2; 
    uint16_t s2:2; 

    MouseAnalogCode mouse_x;
    MouseAnalogCode mouse_y;
    MouseAnalogCode mouse_z;
    bool mouse_left_pressed;
    bool mouse_right_pressed;
    KeySpecifiers key_specifiers;
    uint16_t __resv__;

    [[nodiscard]] std::span<const uint8_t> as_bytes() const noexcept {
        return {reinterpret_cast<const uint8_t*>(this), sizeof(Self)};
    }

    [[nodiscard]] std::span<uint8_t> as_bytes_mut() noexcept {
        return {reinterpret_cast<uint8_t*>(this), sizeof(Self)};
    }
};
#pragma pack(pop)

static_assert(__builtin_offsetof(FrameContext, mouse_x) == 6);
static_assert(__builtin_offsetof(FrameContext, mouse_y) == 8);
static_assert(__builtin_offsetof(FrameContext, mouse_z) == 10);
static_assert(__builtin_offsetof(FrameContext, mouse_left_pressed) == 12);
static_assert(__builtin_offsetof(FrameContext, mouse_right_pressed) == 13);
static_assert(__builtin_offsetof(FrameContext, key_specifiers) == 14);
static_assert(__builtin_offsetof(FrameContext, __resv__) == 16);

static_assert(sizeof(FrameContext) == 18);


};