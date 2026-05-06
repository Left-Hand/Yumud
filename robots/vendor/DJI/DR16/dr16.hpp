#pragma once

#include "hal/conn/uart/hw_singleton.hpp"


namespace ymd::robots::dji::dr16{


#define DR16_KEY_SPECIFIER(KEY_NAME, BIT_INDEX)                     \
    template <typename Self>                                             \
    [[nodiscard]] constexpr auto KEY_NAME(this Self&& self) {            \
        return make_bitfield_proxy<BIT_INDEX, BIT_INDEX + 1, bool>(self.bits);       \
    }

struct alignas(2) [[nodiscard]] KeySpecifiers final {
    uint16_t bits;

    DR16_KEY_SPECIFIER(key_w,     0)
    DR16_KEY_SPECIFIER(key_a,     1)
    DR16_KEY_SPECIFIER(key_s,     2)
    DR16_KEY_SPECIFIER(key_d,     3)
    DR16_KEY_SPECIFIER(key_q,     4)
    DR16_KEY_SPECIFIER(key_e,     5)
    DR16_KEY_SPECIFIER(key_shift, 6)
    DR16_KEY_SPECIFIER(key_ctrl,  7)
};

#undef DR16_KEY_SPECIFIER


#pragma pack(push, 1)
struct [[nodiscard]] alignas(2) FrameContext final{
    using Self =  FrameContext;



    uint16_t ch0:11; //[11 : 0]
    uint16_t ch1:11; //[22 : 11]
    uint16_t ch2:11; //[33 : 22]
    uint16_t ch3:11; //[44 : 33]
    uint16_t s1:2;  //[46 : 44]
    uint16_t s2:2;  //[48 : 46]

    uint16_t mouse_x; //[64 : 48]
    uint16_t mouse_y; //[80 : 64]
    uint16_t mouse_z; //[96 : 80]
    uint8_t mouse_left; //[104 : 96]
    uint8_t mouse_right; //[112 : 104]
    KeySpecifiers key_specifiers;
    uint16_t __resv__;

    [[nodiscard]] imconstexpr std::span<const uint8_t> as_bytes() const noexcept {
        return {reinterpret_cast<const uint8_t*>(this), sizeof(Self)};
    }

    [[nodiscard]] imconstexpr std::span<uint8_t> as_bytes_mut() noexcept {
        return {reinterpret_cast<uint8_t*>(this), sizeof(Self)};
    }
};
#pragma pack(pop)

static_assert(__builtin_offsetof(FrameContext, mouse_x) == 6);
static_assert(__builtin_offsetof(FrameContext, mouse_y) == 8);
static_assert(__builtin_offsetof(FrameContext, mouse_z) == 10);
static_assert(__builtin_offsetof(FrameContext, mouse_left) == 12);
static_assert(__builtin_offsetof(FrameContext, mouse_right) == 13);
static_assert(__builtin_offsetof(FrameContext, key_specifiers) == 14);
static_assert(__builtin_offsetof(FrameContext, __resv__) == 16);

static_assert(sizeof(FrameContext) == 18);


};