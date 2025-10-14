#ifndef __PERIPH_DEFS_H__

#define __PERIPH_DEFS_H__

#include "user_defs.h"

#ifdef CH32V20X
    // #define ENABLE_SDI
    #define ENABLE_GPIOA
    #define ENABLE_GPIOB
    #define ENABLE_GPIOC
    #define ENABLE_GPIOD
    #define ENABLE_DMA1

#elif defined(CH32V30X)
    #define ENABLE_SDI
    #define ENABLE_GPIOA
    #define ENABLE_GPIOB
    #define ENABLE_GPIOC
    #define ENABLE_GPIOD
    #define ENABLE_GPIOE
    #define ENABLE_DVP
    #define ENABLE_RNG
    #define ENABLE_DMA1
    #define ENABLE_DMA2
#endif


#ifdef ENABLE_UART1
    #define UART1_IT_PP 1
    #define UART1_IT_SP 0

    #define UART1_TX_DMA_CH hal::dma1_ch4
    #define UART1_RX_DMA_CH hal::dma1_ch5

    #if UART1_REMAP == 0

    #define UART1_TX_GPIO hal::PA<9>()
    #define UART1_RX_GPIO hal::PA<10>()

    #elif UART1_REMAP == 1

    #define UART1_TX_GPIO hal::PB<6>()
    #define UART1_RX_GPIO hal::PB<7>()

    #endif
#endif


#ifdef ENABLE_UART2
    #define UART2_IT_PP 0
    #define UART2_IT_SP 1

    #define UART2_TX_DMA_CH hal::dma1_ch7
    #define UART2_RX_DMA_CH hal::dma1_ch6

    #if UART2_REMAP == 0

    #define UART2_TX_GPIO hal::PA<2>()
    #define UART2_RX_GPIO hal::PA<3>()

    #elif UART2_REMAP == 1

    #define UART2_TX_GPIO hal::PD<5>()
    #define UART2_RX_GPIO hal::PD<6>()

    #endif
#endif

#ifdef ENABLE_UART3
    #define UART3_IT_PP 1
    #define UART3_IT_SP 1

    #define UART3_TX_DMA_CH hal::dma1_ch2
    #define UART3_RX_DMA_CH hal::dma1_ch3

    #if UART3_REMAP == 0
    #define UART3_TX_GPIO hal::PB<10>()
    #define UART3_RX_GPIO hal::PB<11>()

    #elif UART3_REMAP == 1
    #define UART3_TX_GPIO hal::PC<10>()
    #define UART3_RX_GPIO hal::PC<11>()

    #elif UART3_REMAP == 2
    #define UART3_TX_GPIO hal::PA<13>()
    #define UART3_RX_GPIO hal::PA<14>()

    #elif UART3_REMAP == 3
    #define UART3_TX_GPIO hal::PD<8>()
    #define UART3_RX_GPIO hal::PD<9>()

    #endif
#endif

#ifdef ENABLE_UART4
    #define UART4_IT_PP 1
    #define UART4_IT_SP 1

    #define UART4_TX_DMA_CH hal::dma2_ch5
    #define UART4_RX_DMA_CH hal::dma2_ch3

    #if UART4_REMAP == 0
    #define UART4_TX_GPIO hal::PC<10>()
    #define UART4_RX_GPIO hal::PC<11>()

    #elif UART4_REMAP == 1
    #define UART4_TX_GPIO hal::PB<0>()
    #define UART4_RX_GPIO hal::PB<1>()

    #elif UART4_REMAP == 2
    #define UART4_TX_GPIO hal::PE<0>()
    #define UART4_RX_GPIO hal::PE<1>()

    #elif UART4_REMAP == 3
    #define UART4_TX_GPIO hal::PE<0>()
    #define UART4_RX_GPIO hal::PE<1>()

    #endif
#endif

#ifdef ENABLE_UART5
    #define UART5_IT_PP 1
    #define UART5_IT_SP 1

    #define UART5_TX_DMA_CH hal::dma2_ch4
    #define UART5_RX_DMA_CH hal::dma2_ch2

    #if UART5_REMAP == 0
    #define UART5_TX_GPIO hal::PC<12>()
    #define UART5_RX_GPIO hal::PD<2>()

    #elif UART5_REMAP == 1
    #define UART5_TX_GPIO hal::PB<4>()
    #define UART5_RX_GPIO hal::PB<5>()

    #else
    error uart5 1\x rm not supported
    #endif
#endif



#ifdef ENABLE_UART6
    #define UART6_IT_PP 0
    #define UART6_IT_SP 1

    #define UART6_TX_DMA_CH hal::dma2_ch6
    #define UART6_RX_DMA_CH hal::dma2_ch7

    #if UART6_REMAP == 0
    #define UART6_TX_GPIO hal::PC<0>()
    #define UART6_RX_GPIO hal::PC<1>()

    #elif UART6_REMAP == 1
    #define UART6_TX_GPIO hal::PB<8>()
    #define UART6_RX_GPIO hal::PB<9>()

    #elif (UART6_REMAP == 2) || (UART6_REMAP == 3)
    #define UART6_TX_GPIO hal::PE<10>()
    #define UART6_RX_GPIO hal::PE<11>()

    #endif
#endif

#ifdef ENABLE_UART7
    #define UART7_IT_PP 0
    #define UART7_IT_SP 1

    #define UART7_TX_DMA_CH hal::dma2_ch8
    #define UART7_RX_DMA_CH hal::dma2_ch9

    #if UART7_REMAP == 0
    #define UART7_TX_GPIO hal::PC<2>()
    #define UART7_RX_GPIO hal::PC<3>()

    #elif UART7_REMAP == 1
    #define UART7_TX_GPIO hal::PA<6>()
    #define UART7_RX_GPIO hal::PA<7>()

    #elif (UART7_REMAP == 2) || (UART7_REMAP == 3)
    #define UART7_TX_GPIO hal::PE<12>()
    #define UART7_RX_GPIO hal::PE<13>()

    #endif
#endif


#ifdef ENABLE_UART8
    #define UART8_IT_PP 1
    #define UART8_IT_SP 1

    #define UART8_TX_DMA_CH hal::dma2_ch10
    #define UART8_RX_DMA_CH hal::dma2_ch11

    #if UART8_REMAP == 0
    #define UART8_TX_GPIO hal::PC<4>()
    #define UART8_RX_GPIO hal::PC<5>()

    #elif UART8_REMAP == 1
    #define UART8_TX_GPIO hal::PA<14>()
    #define UART8_RX_GPIO hal::PA<15>()

    #elif (UART8_REMAP == 2) || (UART8_REMAP == 3)
    #define UART8_TX_GPIO hal::PE<14>()
    #define UART8_RX_GPIO hal::PE<15>()

    #endif
#endif




#ifdef ENABLE_SPI1

    #if SPI1_REMAP == 0
        #define SPI1_CS_GPIO hal::PA<4>()
        #define SPI1_SCLK_GPIO hal::PA<5>()
        #define SPI1_MISO_GPIO hal::PA<6>()
        #define SPI1_MOSI_GPIO hal::PA<7>()

    #elif SPI1_REMAP == 1
        #define SPI1_CS_GPIO hal::PA<15>()
        #define SPI1_SCLK_GPIO hal::PB<3>()
        #define SPI1_MISO_GPIO hal::PB<4>()
        #define SPI1_MOSI_GPIO hal::PB<5>()
    #endif
#endif


#ifdef ENABLE_SPI2
    #define SPI2_CS_GPIO hal::PB<12>()
    #define SPI2_SCLK_GPIO hal::PB<13>()
    #define SPI2_MISO_GPIO hal::PB<14>()
    #define SPI2_MOSI_GPIO hal::PB<15>()
#endif



#ifdef ENABLE_I2C1
    #if I2C1_REMAP == 0

        #define I2C1_SCL_GPIO hal::PB<6>()
        #define I2C1_SDA_GPIO hal::PB<7>()

    #elif I2C1_REMAP == 1

        #define I2C1_SCL_GPIO hal::PB<8>()
        #define I2C1_SDA_GPIO hal::PB<9>()

    #endif
#endif


#ifdef ENABLE_I2C2

    #if I2C2_REMAP == 0

    #define I2C2_SCL_GPIO hal::PB<10>()
    #define I2C2_SDA_GPIO hal::PB<11>()

    #endif
#endif




#ifdef ENABLE_TIM1
    #define TIM1_CH1_DMA_CH hal::dma1_ch2
    #define TIM1_CH2_DMA_CH hal::dma1_ch3
    #define TIM1_CH3_DMA_CH hal::dma1_ch6
    #define TIM1_CH4_DMA_CH hal::dma1_ch4

    #if TIM1_REMAP == 0
        #define TIM1_CH1_GPIO hal::PA<8>()
        #define TIM1_CH1N_GPIO hal::PB<13>()

        #define TIM1_CH2_GPIO hal::PA<9>()
        #define TIM1_CH2N_GPIO hal::PB<14>()

        #define TIM1_CH3_GPIO hal::PA<10>()
        #define TIM1_CH3N_GPIO hal::PB<15>()

        #define TIM1_CH4_GPIO hal::PA<11>()
    #elif TIM1_REMAP == 1
        #define TIM1_CH1_GPIO hal::PA<8>()
        #define TIM1_CH1N_GPIO hal::PA<7>()

        #define TIM1_CH2_GPIO hal::PA<9>()
        #define TIM1_CH2N_GPIO hal::PB<0>()

        #define TIM1_CH3_GPIO hal::PA<10>()
        #define TIM1_CH3N_GPIO hal::PB<1>()

        #define TIM1_CH4_GPIO hal::PA<11>()
    #elif TIM1_REMAP == 2
        #error TIM1 remap 2 is preserved
    #elif TIM1_REMAP == 3
        #define TIM1_CH1_GPIO hal::PE<9>()
        #define TIM1_CH1N_GPIO hal::PE<8>()

        #define TIM1_CH2_GPIO hal::PE<11>()
        #define TIM1_CH2N_GPIO hal::PE<10>()

        #define TIM1_CH3_GPIO hal::PE<13>()
        #define TIM1_CH3N_GPIO hal::PE<12>()

        #define TIM1_CH4_GPIO hal::PE<14>()
    #endif
#endif


#ifdef ENABLE_TIM2
    #define TIM2_CH1_DMA_CH hal::dma1_ch5
    #define TIM2_CH2_DMA_CH hal::dma1_ch7
    #define TIM2_CH3_DMA_CH hal::dma1_ch1
    #define TIM2_CH4_DMA_CH hal::dma1_ch7

    #if TIM2_REMAP == 0
        #define TIM2_CH1_GPIO hal::PA<0>()
        #define TIM2_CH2_GPIO hal::PA<1>()
        #define TIM2_CH3_GPIO hal::PA<2>()
        #define TIM2_CH4_GPIO hal::PA<3>()
    #elif TIM2_REMAP == 1
        #define TIM2_CH1_GPIO hal::PA<15>()
        #define TIM2_CH2_GPIO hal::PB<3>()
        #define TIM2_CH3_GPIO hal::PA<2>()
        #define TIM2_CH4_GPIO hal::PA<3>()
    #elif TIM2_REMAP == 2
        #define TIM2_CH1_GPIO hal::PA<0>()
        #define TIM2_CH2_GPIO hal::PA<1>()
        #define TIM2_CH3_GPIO hal::PB<10>()
        #define TIM2_CH4_GPIO hal::PB<11>()
    #elif TIM2_REMAP == 3
        #define TIM2_CH1_GPIO hal::PA<15>()
        #define TIM2_CH2_GPIO hal::PB<3>()
        #define TIM2_CH3_GPIO hal::PB<10>()
        #define TIM2_CH4_GPIO hal::PB<11>()
    #endif
#endif


#ifdef ENABLE_TIM3
    #define TIM3_CH1_DMA_CH hal::dma1_ch6
    #define TIM3_CH3_DMA_CH hal::dma1_ch2
    #define TIM3_CH4_DMA_CH hal::dma1_ch3

    #if TIM3_REMAP == 0
        #define TIM3_CH1_GPIO hal::PA<6>()
        #define TIM3_CH2_GPIO hal::PA<7>()
        #define TIM3_CH3_GPIO hal::PB<0>()
        #define TIM3_CH4_GPIO hal::PB<1>()
    #elif TIM3_REMAP == 1
        #error TIM3 remap 1 is preserved
    #elif TIM3_REMAP == 2
        #define TIM3_CH1_GPIO hal::PB<4>()
        #define TIM3_CH2_GPIO hal::PB<5>()
        #define TIM3_CH3_GPIO hal::PB<0>()
        #define TIM3_CH4_GPIO hal::PB<1>()
    #elif TIM3_REMAP == 3
        #error TIM3 remap 3 is imcomplete
    #endif
#endif

#ifdef ENABLE_TIM4
    #define TIM4_CH1_DMA_CH hal::dma1_ch1
    #define TIM4_CH2_DMA_CH hal::dma1_ch4
    #define TIM4_CH3_DMA_CH hal::dma1_ch5

    #if TIM4_REMAP == 0
        #define TIM4_CH1_GPIO hal::PB<6>()
        #define TIM4_CH2_GPIO hal::PB<7>()
        #define TIM4_CH3_GPIO hal::PB<8>()
        #define TIM4_CH4_GPIO hal::PB<9>()
    #elif TIM4_REMAP == 1
        #define TIM4_CH1_GPIO hal::PD<12>()
        #define TIM4_CH2_GPIO hal::PD<13>()
        #define TIM4_CH3_GPIO hal::PD<14>()
        #define TIM4_CH4_GPIO hal::PD<15>()
    #endif
#endif


#ifdef ENABLE_TIM5
    #define TIM5_CH1_DMA_CH hal::dma2_ch5
    #define TIM5_CH2_DMA_CH hal::dma2_ch4
    #define TIM5_CH3_DMA_CH hal::dma2_ch2
    #define TIM5_CH4_DMA_CH hal::dma2_ch1

    #if TIM5_REMAP == 0
        #define TIM5_CH1_GPIO hal::PA<0>()
        #define TIM5_CH2_GPIO hal::PA<1>()
        #define TIM5_CH3_GPIO hal::PA<2>()
        #define TIM5_CH4_GPIO hal::PA<3>()
    #endif
#endif

#ifdef ENABLE_TIM6
    //TIM6 no io channel
#endif

#ifdef ENABLE_TIM7
    //TIM7 no io channel 
#endif

#ifdef ENABLE_TIM8

    #define TIM8_CH1_DMA_CH hal::dma2_ch3
    #define TIM8_CH2_DMA_CH hal::dma2_ch5
    #define TIM8_CH3_DMA_CH hal::dma2_ch1
    #define TIM8_CH4_DMA_CH hal::dma2_ch2

    #if TIM8_REMAP == 0
        #define TIM8_CH1_GPIO hal::PC<6>()
        #define TIM8_CH1N_GPIO hal::PA<7>()

        #define TIM8_CH2_GPIO hal::PC<7>()
        #define TIM8_CH2N_GPIO hal::PB<0>()

        #define TIM8_CH3_GPIO hal::PC<8>()
        #define TIM8_CH3N_GPIO hal::PB<1>()

        #define TIM8_CH4_GPIO hal::PC<9>()
    #elif TIM8_REMAP == 1
        #define TIM8_CH1_GPIO hal::PB<6>()
        #define TIM8_CH1N_GPIO hal::PA<13>()

        #define TIM8_CH2_GPIO hal::PB<7>()
        #define TIM8_CH2N_GPIO hal::PA<14>()

        #define TIM8_CH3_GPIO hal::PB<8>()
        #define TIM8_CH3N_GPIO hal::PA<15>()

        #define TIM8_CH4_GPIO hal::PC<13>()
    #endif
#endif

#ifdef ENABLE_TIM9
    #define TIM9_CH1_DMA_CH hal::dma2_ch7
    #define TIM9_CH2_DMA_CH hal::dma2_ch9
    #define TIM9_CH3_DMA_CH hal::dma2_ch11
    #define TIM9_CH4_DMA_CH hal::dma2_ch8

    #if TIM9_REMAP == 0
        #define TIM9_CH1_GPIO hal::PA<2>()
        #define TIM9_CH1N_GPIO hal::PC<0>()

        #define TIM9_CH2_GPIO hal::PA<3>()
        #define TIM9_CH2N_GPIO hal::PC<1>()

        #define TIM9_CH3_GPIO hal::PA<4>()
        #define TIM9_CH3N_GPIO hal::PC<2>()

        #define TIM9_CH4_GPIO hal::PC<4>()
    #elif TIM9_REMAP == 1
        #define TIM9_CH1_GPIO hal::PA<2>()
        #define TIM9_CH1N_GPIO hal::PB<0>()

        #define TIM9_CH2_GPIO hal::PA<3>()
        #define TIM9_CH2N_GPIO hal::PB<1>()

        #define TIM9_CH3_GPIO hal::PA<4>()
        #define TIM9_CH3N_GPIO hal::PB<2>()

        #define TIM9_CH4_GPIO hal::PC<14>()
    #elif TIM9_REMAP == 2
        #define TIM9_CH1_GPIO hal::PD<9>()
        #define TIM9_CH1N_GPIO hal::PD<8>()

        #define TIM9_CH2_GPIO hal::PD<11>()
        #define TIM9_CH2N_GPIO hal::PD<10>()

        #define TIM9_CH3_GPIO hal::PD<13>()
        #define TIM9_CH3N_GPIO hal::PD<12>()

        #define TIM9_CH4_GPIO hal::PD<15>()
    #elif TIM9_REMAP == 3
        #define TIM9_CH1_GPIO hal::PD<9>()
        #define TIM9_CH1N_GPIO hal::PD<8>()

        #define TIM9_CH2_GPIO hal::PD<11>()
        #define TIM9_CH2N_GPIO hal::PD<10>()

        #define TIM9_CH3_GPIO hal::PD<13>()
        #define TIM9_CH3N_GPIO hal::PD<12>()

        #define TIM9_CH4_GPIO hal::PD<15>()
    #endif
#endif

#ifdef ENABLE_TIM10
    #define TIM10_CH1_DMA_CH hal::dma2_ch8
    #define TIM10_CH2_DMA_CH hal::dma2_ch10
    #define TIM10_CH3_DMA_CH hal::dma2_ch9
    #define TIM10_CH4_DMA_CH hal::dma2_ch6

    #if TIM10_REMAP == 0
        #define TIM10_CH1_GPIO hal::PB<8>()
        #define TIM10_CH1N_GPIO hal::PA<12>()

        #define TIM10_CH2_GPIO hal::PB<9>()
        #define TIM10_CH2N_GPIO hal::PA<13>()

        #define TIM10_CH3_GPIO hal::PC<3>()
        #define TIM10_CH3N_GPIO hal::PA<14>()

        #define TIM10_CH4_GPIO hal::PC<11>()
    #elif TIM10_REMAP == 1
        #define TIM10_CH1_GPIO hal::PB<3>()
        #define TIM10_CH1N_GPIO hal::PA<5>()

        #define TIM10_CH2_GPIO hal::PB<4>()
        #define TIM10_CH2N_GPIO hal::PA<6>()

        #define TIM10_CH3_GPIO hal::PB<5>()
        #define TIM10_CH3N_GPIO hal::PA<7>()

        #define TIM10_CH4_GPIO hal::PC<14>()
    #elif TIM10_REMAP == 2
        #define TIM10_CH1_GPIO hal::PD<1>()
        #define TIM10_CH1N_GPIO hal::PE<3>()

        #define TIM10_CH2_GPIO hal::PD<3>()
        #define TIM10_CH2N_GPIO hal::PE<4>()

        #define TIM10_CH3_GPIO hal::PD<5>()
        #define TIM10_CH3N_GPIO hal::PE<5>()

        #define TIM10_CH4_GPIO hal::PD<7>()
    #elif TIM10_REMAP == 3
        #define TIM10_CH1_GPIO hal::PD<1>()
        #define TIM10_CH1N_GPIO hal::PE<3>()

        #define TIM10_CH2_GPIO hal::PD<3>()
        #define TIM10_CH2N_GPIO hal::PE<4>()

        #define TIM10_CH3_GPIO hal::PD<5>()
        #define TIM10_CH3N_GPIO hal::PE<5>()

        #define TIM10_CH4_GPIO hal::PD<7>()
    #endif
#endif


#ifdef ENABLE_CAN1
    #define CAN1_RM0_TX_GPIO hal::PA<12>()
    #define CAN1_RM0_RX_GPIO hal::PA<11>()

    #define CAN1_RM1_TX_GPIO hal::PB<9>()
    #define CAN1_RM1_RX_GPIO hal::PB<8>()

    #define CAN1_RM3_TX_GPIO hal::PD<1>()
    #define CAN1_RM3_RX_GPIO hal::PD<0>()
    
    #if CAN1_REMAP == 0
        #define CAN1_TX_GPIO CAN1_RM0_TX_GPIO
        #define CAN1_RX_GPIO CAN1_RM0_RX_GPIO

    #elif CAN1_REMAP == 1
        #define CAN1_TX_GPIO CAN1_RM1_TX_GPIO
        #define CAN1_RX_GPIO CAN1_RM1_RX_GPIO

    #elif CAN1_REMAP == 2
        #warn no can1 remap2

    #elif CAN1_REMAP == 3
        #define CAN1_TX_GPIO CAN1_RM3_TX_GPIO
        #define CAN1_RX_GPIO CAN1_RM3_RX_GPIO

    #endif
#endif

#ifdef ENABLE_CAN2
    #define CAN2_RM0_TX_GPIO hal::PB<13>()
    #define CAN2_RM0_RX_GPIO hal::PB<12>()

    #define CAN2_RM1_TX_GPIO hal::PB<6>()
    #define CAN2_RM1_RX_GPIO hal::PB<5>()
    
    #if CAN2_REMAP == 0
        #define CAN2_TX_GPIO CAN2_RM0_TX_GPIO
        #define CAN2_RX_GPIO CAN2_RM0_RX_GPIO

    #elif CAN2_REMAP == 1
        #define CAN2_TX_GPIO CAN2_RM1_TX_GPIO
        #define CAN2_RX_GPIO CAN2_RM1_RX_GPIO

    #endif
#endif

#ifdef ENABLE_OPA2
    #define OPA2_N0_GPIO hal::PB<10>()
    #define OPA2_P0_GPIO hal::PB<14>()
    #define OPA2_O0_GPIO hal::PA<2>()

    #define OPA2_N1_GPIO hal::PA<5>()
    #define OPA2_P1_GPIO hal::PA<7>()
    #define OPA2_O1_GPIO hal::PA<4>()

#endif

#ifdef ENABLE_OPA1
    #define OPA1_N0_GPIO hal::PB<11>()
    #define OPA1_P0_GPIO hal::PB<15>()
    #define OPA1_O0_GPIO hal::PA<3>()

    #define OPA1_N1_GPIO hal::PA<6>()
    #define OPA1_P1_GPIO hal::PB<0>()
    #define OPA1_O1_GPIO hal::PB<1>()
#endif


#endif