#pragma once

#include <cstdint>

#define QKV4_REG_DEF(name) static inline volatile name##_Def * name = (name##_Def *)name##_Def::address;

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
            uint16_t lck:16;
            uint16_t lckk:1;
            uint16_t 15;
        };

        CFGLR cfglr;
        CFGHR cfghr;
        INDR indr;
        OUTDR outdr;
        BSHR bshr;
        BCR bcr;
        LCKR lckr;
    };

    static inline volatile GPIO_REG_Def gpio_a_def = (volatile GPIO_REG_Def *)(0x40010800);
    static inline volatile GPIO_REG_Def gpio_b_def = (volatile GPIO_REG_Def *)(0x40010C00);
    static inline volatile GPIO_REG_Def gpio_c_def = (volatile GPIO_REG_Def *)(0x40011000);
    static inline volatile GPIO_REG_Def gpio_d_def = (volatile GPIO_REG_Def *)(0x40011400);
    static inline volatile GPIO_REG_Def gpio_e_def = (volatile GPIO_REG_Def *)(0x40011800);
}