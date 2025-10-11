#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp" 
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct TCA8418_Prelude{
    using RegAddr = uint8_t;

    enum class Error_Kind:uint8_t{
        VerifyFailed,
        WrongWhoAmI
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
};
struct TCA8418_Regs:public TCA8418_Prelude{
    struct R8_Cfg:public Reg8<>{
        static constexpr RegAddr address = 0x01;

        // Key events interrupt enable to host processor
        uint8_t KE_IEN:1;

        // GPI interrupt enable to host processor
        uint8_t GPI_IEN:1;

        // Keypad lock interrupt enable
        uint8_t K_LCK_IEN:1;

        // Overflow interrupt enable
        uint8_t OVR_FLOW_EN:1;

        // Interrupt configuration
        uint8_t IN_CFG:1;

        // Overflow mode
        uint8_t OVR_FLOW_M:1;

        // GPI event mode configuration
        uint8_t GPI_E_CFG:1;

        // GPI event mode configuration
        uint8_t AI:1;
    }DEF_R8(cfg_reg)

    struct R8_INT_STAT:public Reg8<>{
        static constexpr RegAddr address = 0x02;

        uint8_t K_INT:1;
        uint8_t GPI_INT:1;
        uint8_t K_CLK_INT:1;
        uint8_t OVGR_FLOW_INT:1;
        uint8_t CAD_INT:1;
        uint8_t :3;
    }DEF_R8(int_stat_reg)

    struct R8_KEY_LCK_EC:public Reg8<>{
        static constexpr RegAddr address = 0x03;

        uint8_t KEC:4;
        uint8_t LCK1:1;
        uint8_t LCK2:1;
        uint8_t K_LCK_EN:1;
        uint8_t __RESV__:1;
    }DEF_R8(key_lck_ec_reg)

    struct R8_KEY_EVENT:public Reg8<>{
        static constexpr RegAddr address = 0x04;

        uint8_t data;
    }DEF_R8(key_event_reg)

    struct R8_KP_LCK_TIMER:public Reg8<>{
        static constexpr RegAddr address = 0x05;

        uint8_t data;
    }DEF_R8(kp_lck_timer_reg)

    struct R8_KP_UNLOCK_TIMER:public Reg8<>{
        static constexpr RegAddr address = 0x05;

        uint8_t data;
    }DEF_R8(kp_unlock_timer_reg)

    struct R8_GPIO_INT_STAT1:public Reg8<>{
        static constexpr RegAddr address = 0x11;

        uint8_t R0IS:1;
        uint8_t R1IS:1;
        uint8_t R2IS:1;
        uint8_t R3IS:1;
        uint8_t R4IS:1;
        uint8_t R5IS:1;
        uint8_t R6IS:1;
        uint8_t R7IS:1;
    }DEF_R8(gpio_int_stat1_reg)

    struct R8_GPIO_INT_STAT2:public Reg8<>{
        static constexpr RegAddr address = 0x12;

        uint8_t C0IS:1;
        uint8_t C1IS:1;
        uint8_t C2IS:1;
        uint8_t C3IS:1;
        uint8_t C4IS:1;
        uint8_t C5IS:1;
        uint8_t C6IS:1;
        uint8_t C7IS:1;
    }DEF_R8(gpio_int_stat2_reg)

    struct R8_GPIO_INT_STAT3:public Reg8<>{
        static constexpr RegAddr address = 0x13;

        uint8_t C8IS:1;
        uint8_t C9IS:1;
        uint8_t __RESV__:6;
    }DEF_R8(gpio_int_stat3_reg)

    struct R8_GPIO_DAT_STAT1:public Reg8<>{
        static constexpr RegAddr address = 0x14;

        uint8_t R0DS:1;
        uint8_t R1DS:1;
        uint8_t R2DS:1;
        uint8_t R3DS:1;
        uint8_t R4DS:1;
        uint8_t R5DS:1;
        uint8_t R6DS:1;
        uint8_t R7DS:1;
    }DEF_R8(gpio_dat_stat1_reg)

    struct R8_GPIO_DAT_STAT2:public Reg8<>{
        static constexpr RegAddr address = 0x15;

        uint8_t C0DS:1;
        uint8_t C1DS:1;
        uint8_t C2DS:1;
        uint8_t C3DS:1;
        uint8_t C4DS:1;
        uint8_t C5DS:1;
        uint8_t C6DS:1;
        uint8_t C7DS:1;
    }DEF_R8(gpio_dat_stat2_reg)

    struct R8_GPIO_DAT_STAT3:public Reg8<>{
        static constexpr RegAddr address = 0x16;

        uint8_t C8DS:1;
        uint8_t C9DS:1;
        uint8_t :6;
    }DEF_R8(gpio_dat_stat3_reg)

    struct R8_GPIO_DAT_OUT1:public Reg8<>{
        static constexpr RegAddr address = 0x17;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(gpio_dat_out1_reg)

    struct R8_GPIO_DAT_OUT2:public Reg8<>{
        static constexpr RegAddr address = 0x18;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(gpio_dat_out2_reg)

    struct R8_GPIO_DAT_OUT3:public Reg8<>{
        static constexpr RegAddr address = 0x19;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(gpio_dat_out3_reg)

    struct R8_GPIO_INT_EN1:public Reg8<>{
        static constexpr RegAddr address = 0x1A;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(gpio_int_en1_reg)

    struct R8_GPIO_INT_EN2:public Reg8<>{
        static constexpr RegAddr address = 0x1B;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(gpio_int_en2_reg)

    struct R8_GPIO_INT_EN3:public Reg8<>{
        static constexpr RegAddr address = 0x1C;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(gpio_int_en3_reg)

    struct R8_KP_GPIO1:public Reg8<>{
        static constexpr RegAddr address = 0x1D;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(kp_gpio1_reg)

    struct R8_KP_GPIO2:public Reg8<>{
        static constexpr RegAddr address = 0x1E;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(kp_gpio2_reg)

    struct R8_KP_GPIO3:public Reg8<>{
        static constexpr RegAddr address = 0x1F;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(kp_gpio3_reg)


    struct R8_GPI_EM1:public Reg8<>{
        static constexpr RegAddr address = 0x20;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(kp_gpio4_reg)

    struct R8_GPI_EM2:public Reg8<>{
        static constexpr RegAddr address = 0x21;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(kp_gpio5_reg)

    struct R8_GPI_EM3:public Reg8<>{
        static constexpr RegAddr address = 0x22;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(kp_gpio6_reg)

    struct R8_GPIO_DIR1:public Reg8<>{
        static constexpr RegAddr address = 0x23;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(gpio_dir1_reg)

    struct R8_GPIO_DIR2:public Reg8<>{
        static constexpr RegAddr address = 0x24;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(gpio_dir2_reg)

    struct R8_GPIO_DIR3:public Reg8<>{
        static constexpr RegAddr address = 0x25;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(gpio_dir3_reg)

    struct R8_GPIO_INT_LVL1:public Reg8<>{
        static constexpr RegAddr address = 0x26;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(gpio_int_lvl1_reg)

    struct R8_GPIO_INT_LVL2:public Reg8<>{
        static constexpr RegAddr address = 0x27;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(gpio_int_lvl2_reg)

    struct R8_GPIO_INT_LVL3:public Reg8<>{
        static constexpr RegAddr address = 0x28;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(gpio_int_lvl3_reg)

    struct R8_DEBOUNCE_DIS1:public Reg8<>{
        static constexpr RegAddr address = 0x29;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(debounce_dis1_reg)

    struct R8_DEBOUNCE_DIS2:public Reg8<>{
        static constexpr RegAddr address = 0x2A;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(debounce_dis2_reg)

    struct R8_DEBOUNCE_DIS3:public Reg8<>{
        static constexpr RegAddr address = 0x2B;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(debounce_dis3_reg)

    struct R8_GPIO_PULL1:public Reg8<>{
        static constexpr RegAddr address = 0x2C;

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(gpio_pull1_reg)

    struct R8_GPIO_PULL2:public Reg8<>{
        static constexpr RegAddr address = 0x2D;

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(gpio_pull2_reg)

    struct R8_GPIO_PULL3:public Reg8<>{
        static constexpr RegAddr address = 0x2E;

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(gpio_pull3_reg)
};

}