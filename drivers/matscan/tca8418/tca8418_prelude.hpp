#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp" 
#include "core/utils/Errno.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"


/**
 *  @file Adafruit_TCA8418_registers.h
 *
 * 	I2C Driver for the Adafruit TCA8418 Keypad Matrix / GPIO Expander
 *Breakout
 *
 * 	This is a library for the Adafruit TCA8418 breakout:
 * 	https://www.adafruit.com/products/4918
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *
 *	BSD license (see license.txt)
 */


namespace ymd::drivers{

struct TCA8418_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b0110100);

    enum class Error_Kind:uint8_t{
        VerifyFailed,
        InvalidChipId
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;


enum class RegAddr:uint8_t{
    CFG = 0x01,             ///< Configuration register
    INT_STAT = 0x02,        ///< Interrupt status
    KEY_LCK_EC = 0x03,      ///< Key lock and event counter
    KEY_EVENT_A = 0x04,     ///< Key event register A
    KEY_EVENT_B = 0x05,     ///< Key event register B
    KEY_EVENT_C = 0x06,     ///< Key event register C
    KEY_EVENT_D = 0x07,     ///< Key event register D
    KEY_EVENT_E = 0x08,     ///< Key event register E
    KEY_EVENT_F = 0x09,     ///< Key event register F
    KEY_EVENT_G = 0x0A,     ///< Key event register G
    KEY_EVENT_H = 0x0B,     ///< Key event register H
    KEY_EVENT_I = 0x0C,     ///< Key event register I
    KEY_EVENT_J = 0x0D,     ///< Key event register J
    KP_LCK_TIMER = 0x0E,    ///< Keypad lock1 to lock2 timer
    UNLOCK_1 = 0x0F,        ///< Unlock register 1
    UNLOCK_2 = 0x10,        ///< Unlock register 2
    GPIO_INT_STAT_1 = 0x11, ///< GPIO interrupt status 1
    GPIO_INT_STAT_2 = 0x12, ///< GPIO interrupt status 2
    GPIO_INT_STAT_3 = 0x13, ///< GPIO interrupt status 3
    GPIO_DAT_STAT_1 = 0x14, ///< GPIO data status 1
    GPIO_DAT_STAT_2 = 0x15, ///< GPIO data status 2
    GPIO_DAT_STAT_3 = 0x16, ///< GPIO data status 3
    GPIO_DAT_OUT_1 = 0x17,  ///< GPIO data out 1
    GPIO_DAT_OUT_2 = 0x18,  ///< GPIO data out 2
    GPIO_DAT_OUT_3 = 0x19,  ///< GPIO data out 3
    GPIO_INT_EN_1 = 0x1A,   ///< GPIO interrupt enable 1
    GPIO_INT_EN_2 = 0x1B,   ///< GPIO interrupt enable 2
    GPIO_INT_EN_3 = 0x1C,   ///< GPIO interrupt enable 3
    KP_GPIO_1 = 0x1D,       ///< Keypad/GPIO select 1
    KP_GPIO_2 = 0x1E,       ///< Keypad/GPIO select 2
    KP_GPIO_3 = 0x1F,       ///< Keypad/GPIO select 3
    GPI_EM_1 = 0x20,        ///< GPI event mode 1
    GPI_EM_2 = 0x21,        ///< GPI event mode 2
    GPI_EM_3 = 0x22,        ///< GPI event mode 3
    GPIO_DIR_1 = 0x23,      ///< GPIO data direction 1
    GPIO_DIR_2 = 0x24,      ///< GPIO data direction 2
    GPIO_DIR_3 = 0x25,      ///< GPIO data direction 3
    GPIO_INT_LVL_1 = 0x26,  ///< GPIO edge/level detect 1
    GPIO_INT_LVL_2 = 0x27,  ///< GPIO edge/level detect 2
    GPIO_INT_LVL_3 = 0x28,  ///< GPIO edge/level detect 3
    DEBOUNCE_DIS_1 = 0x29,  ///< Debounce disable 1
    DEBOUNCE_DIS_2 = 0x2A,  ///< Debounce disable 2
    DEBOUNCE_DIS_3 = 0x2B,  ///< Debounce disable 3
    GPIO_PULL_1 = 0x2C,     ///< GPIO pull-up disable 1
    GPIO_PULL_2 = 0x2D,     ///< GPIO pull-up disable 2
    GPIO_PULL_3 = 0x2E,     ///< GPIO pull-up disable 3
    // #define TCA8418_REG_RESERVED          0x2F

    //  FIELDS CONFIG REGISTER  1
    CFG_AI = 0x80,           ///< Auto-increment for read/write
    CFG_GPI_E_CGF = 0x40,    ///< Event mode config
    CFG_OVR_FLOW_M = 0x20,   ///< Overflow mode enable
    CFG_INT_CFG = 0x10,      ///< Interrupt config
    CFG_OVR_FLOW_IEN = 0x08, ///< Overflow interrupt enable
    CFG_K_LCK_IEN = 0x04,    ///< Keypad lock interrupt enable
    CFG_GPI_IEN = 0x02,      ///< GPI interrupt enable
    CFG_KE_IEN = 0x01,       ///< Key events interrupt enable

    //  FIELDS INT_STAT REGISTER  2
    STAT_CAD_INT = 0x10,      ///< Ctrl-alt-del seq status
    STAT_OVR_FLOW_INT = 0x08, ///< Overflow interrupt status
    STAT_K_LCK_INT = 0x04,    ///< Key lock interrupt status
    STAT_GPI_INT = 0x02,      ///< GPI interrupt status
    STAT_K_INT = 0x01,        ///< Key events interrupt status

    //  FIELDS  KEY_LCK_EC REGISTER 3
    LCK_EC_K_LCK_EN = 0x40, ///< Key lock enable
    LCK_EC_LCK_2 = 0x20,    ///< Keypad lock status 2
    LCK_EC_LCK_1 = 0x10,    ///< Keypad lock status 1
    LCK_EC_KLEC_3 = 0x08,   ///< Key event count bit 3
    LCK_EC_KLEC_2 = 0x04,   ///< Key event count bit 2
    LCK_EC_KLEC_1 = 0x02,   ///< Key event count bit 1
    LCK_EC_KLEC_0 = 0x01,   ///< Key event count bit 0
};
};
struct TCA8418_Regs:public TCA8418_Prelude{
    struct R8_Cfg:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x01};

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

    struct R8_INT_STAT:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x02};

        uint8_t K_INT:1;
        uint8_t GPI_INT:1;
        uint8_t K_CLK_INT:1;
        uint8_t OVGR_FLOW_INT:1;
        uint8_t CAD_INT:1;
        uint8_t :3;
    }DEF_R8(int_stat_reg)

    struct R8_KEY_LCK_EC:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x03};

        uint8_t KEC:4;
        uint8_t LCK1:1;
        uint8_t LCK2:1;
        uint8_t K_LCK_EN:1;
        uint8_t __RESV__:1;
    }DEF_R8(key_lck_ec_reg)

    struct R8_KEY_EVENT:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x04};

        uint8_t data;
    }DEF_R8(key_event_reg)

    struct R8_KP_LCK_TIMER:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x05};

        uint8_t data;
    }DEF_R8(kp_lck_timer_reg)

    struct R8_KP_UNLOCK_TIMER:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x05};

        uint8_t data;
    }DEF_R8(kp_unlock_timer_reg)

    struct R8_GPIO_INT_STAT1:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x11};

        uint8_t R0IS:1;
        uint8_t R1IS:1;
        uint8_t R2IS:1;
        uint8_t R3IS:1;
        uint8_t R4IS:1;
        uint8_t R5IS:1;
        uint8_t R6IS:1;
        uint8_t R7IS:1;
    }DEF_R8(gpio_int_stat1_reg)

    struct R8_GPIO_INT_STAT2:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x12};

        uint8_t C0IS:1;
        uint8_t C1IS:1;
        uint8_t C2IS:1;
        uint8_t C3IS:1;
        uint8_t C4IS:1;
        uint8_t C5IS:1;
        uint8_t C6IS:1;
        uint8_t C7IS:1;
    }DEF_R8(gpio_int_stat2_reg)

    struct R8_GPIO_INT_STAT3:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x13};

        uint8_t C8IS:1;
        uint8_t C9IS:1;
        uint8_t __RESV__:6;
    }DEF_R8(gpio_int_stat3_reg)

    struct R8_GPIO_DAT_STAT1:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x14};

        uint8_t R0DS:1;
        uint8_t R1DS:1;
        uint8_t R2DS:1;
        uint8_t R3DS:1;
        uint8_t R4DS:1;
        uint8_t R5DS:1;
        uint8_t R6DS:1;
        uint8_t R7DS:1;
    }DEF_R8(gpio_dat_stat1_reg)

    struct R8_GPIO_DAT_STAT2:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x15};

        uint8_t C0DS:1;
        uint8_t C1DS:1;
        uint8_t C2DS:1;
        uint8_t C3DS:1;
        uint8_t C4DS:1;
        uint8_t C5DS:1;
        uint8_t C6DS:1;
        uint8_t C7DS:1;
    }DEF_R8(gpio_dat_stat2_reg)

    struct R8_GPIO_DAT_STAT3:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x16};

        uint8_t C8DS:1;
        uint8_t C9DS:1;
        uint8_t :6;
    }DEF_R8(gpio_dat_stat3_reg)

    struct R8_GPIO_DAT_OUT1:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x17};

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(gpio_dat_out1_reg)

    struct R8_GPIO_DAT_OUT2:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x18};

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(gpio_dat_out2_reg)

    struct R8_GPIO_DAT_OUT3:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x19};

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(gpio_dat_out3_reg)

    struct R8_GPIO_INT_EN1:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1A};

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(gpio_int_en1_reg)

    struct R8_GPIO_INT_EN2:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1B};

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(gpio_int_en2_reg)

    struct R8_GPIO_INT_EN3:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1C};

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(gpio_int_en3_reg)

    struct R8_KP_GPIO1:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1D};

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(kp_gpio1_reg)

    struct R8_KP_GPIO2:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1E};

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(kp_gpio2_reg)

    struct R8_KP_GPIO3:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1F};

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(kp_gpio3_reg)


    struct R8_GPI_EM1:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x20};

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(kp_gpio4_reg)

    struct R8_GPI_EM2:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x21};

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(kp_gpio5_reg)

    struct R8_GPI_EM3:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x22};

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(kp_gpio6_reg)

    struct R8_GPIO_DIR1:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x23};

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(gpio_dir1_reg)

    struct R8_GPIO_DIR2:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x24};

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(gpio_dir2_reg)

    struct R8_GPIO_DIR3:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x25};

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(gpio_dir3_reg)

    struct R8_GPIO_INT_LVL1:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x26};

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(gpio_int_lvl1_reg)

    struct R8_GPIO_INT_LVL2:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x27};

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(gpio_int_lvl2_reg)

    struct R8_GPIO_INT_LVL3:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x28};

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(gpio_int_lvl3_reg)

    struct R8_DEBOUNCE_DIS1:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x29};

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(debounce_dis1_reg)

    struct R8_DEBOUNCE_DIS2:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x2A};

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(debounce_dis2_reg)

    struct R8_DEBOUNCE_DIS3:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x2B};

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(debounce_dis3_reg)

    struct R8_GPIO_PULL1:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x2C};

        uint8_t R0DO:1;
        uint8_t R1DO:1;
        uint8_t R2DO:1;
        uint8_t R3DO:1;
        uint8_t R4DO:1;
        uint8_t R5DO:1;
        uint8_t R6DO:1;
        uint8_t R7DO:1;
    }DEF_R8(gpio_pull1_reg)

    struct R8_GPIO_PULL2:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x2D};

        uint8_t C0DO:1;
        uint8_t C1DO:1;
        uint8_t C2DO:1;
        uint8_t C3DO:1;
        uint8_t C4DO:1;
        uint8_t C5DO:1;
        uint8_t C6DO:1;
        uint8_t C7DO:1;
    }DEF_R8(gpio_pull2_reg)

    struct R8_GPIO_PULL3:public Reg8{
        static constexpr RegAddr REG_ADDR = RegAddr{0x2E};

        uint8_t C8DO:1;
        uint8_t C9DO:1;
        uint8_t :6;
    }DEF_R8(gpio_pull3_reg)
};

}