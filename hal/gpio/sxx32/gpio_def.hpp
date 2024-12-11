#pragma once

#include <cstdint>

#define REG_DEF(name) static inline volatile name##_Def * name = (name##_Def *)name##_Def::address;

namespace SXX32{
    struct GPIO_Def{
        struct CFGLR{

        };

        struct CFGHR{

        };

        struct INDR{
            uint32_t idr:16;
            uint32_t :16;
        };

        struct OUTDR{
            uint32_t odr:16;
            uint32_t :16;
        };

        struct BSHR{
            uint32_t bs:16;
            uint32_t br:16;

        };

        struct BCR{
            uint32_t br:16;
            uint32_t :16;
        };

        struct LCKR{
            uint32_t lck:16;
            uint32_t lckk:1;
            uint32_t 15;
        };

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