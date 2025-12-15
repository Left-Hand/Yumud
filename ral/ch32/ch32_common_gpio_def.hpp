#pragma once

#include <cstdint>


namespace ymd::ral::ch32::common_gpio{
struct [[nodiscard]] R32_CFGLR{

};

struct [[nodiscard]] R32_CFGHR{

};

struct [[nodiscard]] R32_INDR{
    uint32_t idr:16;
    uint32_t :16;
};

struct [[nodiscard]] R32_OUTDR{
    uint32_t odr:16;
    uint32_t :16;
};

struct [[nodiscard]] R32_BSHR{
    uint32_t bs:16;
    uint32_t br:16;

};

struct [[nodiscard]] R32_BCR{
    uint32_t br:16;
    uint32_t :16;
};

struct [[nodiscard]] R32_LCKR{
    uint32_t lck:16;
    uint32_t lckk:1;
    uint32_t :15;
};
struct GPIO_Def{

    volatile CFGLR cfglr;
    volatile CFGHR cfghr;
    volatile INDR indr;
    volatile OUTDR outdr;
    volatile BSHR bshr;
    volatile BCR bcr;
    volatile LCKR lckr;
};

static inline GPIO_Def * gpio_a_def = (GPIO_Def *)(0x40010800);
static inline GPIO_Def * gpio_b_def = (GPIO_Def *)(0x40010C00);
static inline GPIO_Def * gpio_c_def = (GPIO_Def *)(0x40011000);
static inline GPIO_Def * gpio_d_def = (GPIO_Def *)(0x40011400);
static inline GPIO_Def * gpio_e_def = (GPIO_Def *)(0x40011800);
}