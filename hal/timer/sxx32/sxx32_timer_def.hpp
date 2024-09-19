#pragma once

#include <cstdint>

namespace SXX32{
    struct TIM_Def{
        struct CTLR1{
            uint16_t cen:1;
            uint16_t udis:1;
            uint16_t urs:1;
            uint16_t opm:1;
            uint16_t dir:1;
            uint16_t cms:2;
            uint16_t apre:1;
            uint16_t ckd:2;
            uint16_t :6;
        };

        struct CFGHR{
            uint16_t ccpc:1;
            uint16_t :1;
            uint16_t ccus:1;
            uint16_t ccds:1;
            uint16_t mms:3;

            uint16_t ti1s:1;
            uint16_t ois1:1;
            uint16_t ois1n:1;
            uint16_t ois2:1;
            uint16_t ois2n:1;
            uint16_t ois3:1;
            uint16_t ois3n:1;
            uint16_t ois4:1;
            uint16_t :1;
        };


    };

    // static inline volatile GPIO_REG_Def timer_1_def = (volatile GPIO_REG_Def *)(0x40010800);
    // static inline volatile GPIO_REG_Def gpio_b_def = (volatile GPIO_REG_Def *)(0x40010C00);
    // static inline volatile GPIO_REG_Def gpio_c_def = (volatile GPIO_REG_Def *)(0x40011000);
    // static inline volatile GPIO_REG_Def gpio_d_def = (volatile GPIO_REG_Def *)(0x40011400);
    // static inline volatile GPIO_REG_Def gpio_e_def = (volatile GPIO_REG_Def *)(0x40011800);
}