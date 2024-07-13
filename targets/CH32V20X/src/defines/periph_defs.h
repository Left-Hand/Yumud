#ifndef __PERIPH_DEFS_H__

#define __PERIPH_DEFS_H__

#include "user_defs.h"

#ifdef CH32V20X
    // #define HAVE_SDI
    #define HAVE_GPIOA
    #define HAVE_GPIOB
    #define HAVE_GPIOC
    #define HAVE_GPIOD
    #define HAVE_DMA1

#elif defined(CH32V30X)
    #define HAVE_SDI
    #define HAVE_GPIOA
    #define HAVE_GPIOB
    #define HAVE_GPIOC
    #define HAVE_GPIOD
    #define HAVE_GPIOE
    #define HAVE_DVP
    #define HAVE_RNG
    #define HAVE_DMA1
    #define HAVE_DMA2
#endif


#ifdef HAVE_UART1
    #define UART1_IT_PP 1
    #define UART1_IT_SP 0

    #define UART1_TX_DMA_CH dma1Ch4
    #define UART1_RX_DMA_CH dma1Ch5

    #if UART1_REMAP == 0

    #define UART1_TX_Gpio portA[9]
    #define UART1_RX_Gpio portA[10]

    #elif UART1_REMAP == 1

    #define UART1_TX_Gpio portB[6]
    #define UART1_RX_Gpio portB[7]

    #endif
#endif


#ifdef HAVE_UART2
    #define UART2_IT_PP 1
    #define UART2_IT_SP 1

    #define UART2_TX_DMA_CH dma1Ch7
    #define UART2_RX_DMA_CH dma1Ch6

    #if UART2_REMAP == 0

    #define UART2_TX_Gpio portA[2]
    #define UART2_RX_Gpio portA[3]

    #elif UART2_REMAP == 1

    #define UART2_TX_Gpio portD[5]
    #define UART2_RX_Gpio portD[6]

    #endif
#endif

#ifdef HAVE_UART3
    #define UART3_IT_PP 1
    #define UART3_IT_SP 1

    #define UART3_TX_DMA_CH dma1Ch2
    #define UART3_RX_DMA_CH dma1Ch3

    #if UART3_REMAP == 0
    #define UART3_TX_Gpio portB[10]
    #define UART3_RX_Gpio portB[11]

    #elif UART3_REMAP == 1
    #define UART3_TX_Gpio portC[10]
    #define UART3_RX_Gpio portC[11]

    #elif UART3_REMAP == 2
    #define UART3_TX_Gpio portA[13]
    #define UART3_RX_Gpio portA[14]

    #elif UART3_REMAP == 3
    #define UART3_TX_Gpio portD[8]
    #define UART3_RX_Gpio portD[9]

    #endif
#endif

#ifdef HAVE_UART4
    #define UART4_IT_PP 1
    #define UART4_IT_SP 1

    #define UART4_TX_DMA_CH dma2Ch5
    #define UART4_RX_DMA_CH dma2Ch3

    #if UART4_REMAP == 0
    #define UART4_TX_Gpio portC[10]
    #define UART4_RX_Gpio portC[11]

    #elif UART4_REMAP == 1
    #define UART4_TX_Gpio portB[0]
    #define UART4_RX_Gpio portB[1]

    #elif UART4_REMAP == 2
    #define UART4_TX_Gpio portE[0]
    #define UART4_RX_Gpio portE[1]

    #elif UART4_REMAP == 3
    #define UART4_TX_Gpio portE[0]
    #define UART4_RX_Gpio portE[1]

    #endif
#endif

#ifdef HAVE_UART5
    #define UART5_IT_PP 1
    #define UART5_IT_SP 1

    #define UART5_TX_DMA_CH dma2Ch4
    #define UART5_RX_DMA_CH dma2Ch2

    #if UART5_REMAP == 0
    #define UART5_TX_Gpio portC[12]
    #define UART5_RX_Gpio portD[2]

    #elif UART5_REMAP == 1
    #define UART5_TX_Gpio portB[4]
    #define UART5_RX_Gpio portB[5]

    #else
    error uart5 1\x rm not supported
    #endif
#endif



#ifdef HAVE_UART6
    #define UART6_IT_PP 1
    #define UART6_IT_SP 1

    #define UART6_TX_DMA_CH dma2Ch6
    #define UART6_RX_DMA_CH dma2Ch7

    #if UART6_REMAP == 0
    #define UART6_TX_Gpio portC[0]
    #define UART6_RX_Gpio portC[1]

    #elif UART6_REMAP == 1
    #define UART6_TX_Gpio portB[8]
    #define UART6_RX_Gpio portB[9]

    #elif (UART6_REMAP == 2) || (UART6_REMAP == 3)
    #define UART6_TX_Gpio portE[10]
    #define UART6_RX_Gpio portE[11]

    #endif
#endif

#ifdef HAVE_UART7
    #define UART7_IT_PP 1
    #define UART7_IT_SP 1

    #define UART7_TX_DMA_CH dma2Ch8
    #define UART7_RX_DMA_CH dma2Ch9

    #if UART7_REMAP == 0
    #define UART7_TX_Gpio portC[2]
    #define UART7_RX_Gpio portC[3]

    #elif UART7_REMAP == 1
    #define UART7_TX_Gpio portA[6]
    #define UART7_RX_Gpio portA[7]

    #elif (UART7_REMAP == 2) || (UART_REMAP == 3)
    #define UART7_TX_Gpio portE[12]
    #define UART7_RX_Gpio portE[13]

    #endif
#endif


#ifdef HAVE_UART8
    #define UART8_IT_PP 1
    #define UART8_IT_SP 1

    #define UART8_TX_DMA_CH dma2Ch10
    #define UART8_RX_DMA_CH dma2Ch11

    #if UART8_REMAP == 0
    #define UART8_TX_Gpio portC[4]
    #define UART8_RX_Gpio portC[5]

    #elif UART8_REMAP == 1
    #define UART8_TX_Gpio portA[14]
    #define UART8_RX_Gpio portA[15]

    #elif (UART8_REMAP == 2) || (UART8_REMAP == 3)
    #define UART8_TX_Gpio portE[14]
    #define UART8_RX_Gpio portE[15]

    #endif
#endif




#ifdef HAVE_SPI1

    #if SPI1_REMAP == 0
        #define SPI1_CS_Gpio portA[4]
        #define SPI1_SCLK_Gpio portA[5]
        #define SPI1_MISO_Gpio portA[6]
        #define SPI1_MOSI_Gpio portA[7]

    #elif SPI1_REMAP == 1
        #define SPI1_CS_Gpio portA[15]
        #define SPI1_SCLK_Gpio portB[3]
        #define SPI1_MISO_Gpio portB[4]
        #define SPI1_MOSI_Gpio portB[5]
    #endif
#endif


#ifdef HAVE_SPI2
    #define SPI2_CS_Gpio portB[12]
    #define SPI2_SCLK_Gpio portB[13]
    #define SPI2_MISO_Gpio portB[14]
    #define SPI2_MOSI_Gpio portB[15]
#endif



#ifdef HAVE_I2C1
    #if I2C1_REMAP == 0

        #define I2C1_SCL_Gpio portB[6]
        #define I2C1_SDA_Gpio portB[7]

    #elif I2C1_REMAP == 1

        #define I2C1_SCL_Gpio portB[8]
        #define I2C1_SDA_Gpio portB[9]

    #endif
#endif


#ifdef HAVE_I2C2

    #if I2C2_REMAP == 0

    #define I2C2_SCL_Gpio portB[10]
    #define I2C2_SDA_Gpio portB[11]

    #endif
#endif




#ifdef HAVE_TIM1
    #if TIM1_REMAP == 0
        #define TIM1_CH1_Gpio portA[8]
        #define TIM1_CH1N_Gpio portB[13]

        #define TIM1_CH2_Gpio portA[9]
        #define TIM1_CH2N_Gpio portB[14]

        #define TIM1_CH3_Gpio portA[10]
        #define TIM1_CH3N_Gpio portB[15]

        #define TIM1_CH4_Gpio portA[11]
    #elif TIM1_REMAP == 1
        #define TIM1_CH1_Gpio portA[8]
        #define TIM1_CH1N_Gpio portA[7]

        #define TIM1_CH2_Gpio portA[9]
        #define TIM1_CH2N_Gpio portB[0]

        #define TIM1_CH3_Gpio portA[10]
        #define TIM1_CH3N_Gpio portB[1]

        #define TIM1_CH4_Gpio portA[11]
    #elif TIM1_REMAP == 2
        #error TIM1 remap 2 is preserved
    #elif TIM1_REMAP == 3
        #define TIM1_CH1_Gpio portE[9]
        #define TIM1_CH1N_Gpio portE[8]

        #define TIM1_CH2_Gpio portE[11]
        #define TIM1_CH2N_Gpio portE[10]

        #define TIM1_CH3_Gpio portE[13]
        #define TIM1_CH3N_Gpio portE[12]

        #define TIM1_CH4_Gpio portE[14]
    #endif
#endif


#ifdef HAVE_TIM2
    #if TIM2_REMAP == 0
        #define TIM2_CH1_Gpio portA[0]
        #define TIM2_CH2_Gpio portA[1]
        #define TIM2_CH3_Gpio portA[2]
        #define TIM2_CH4_Gpio portA[3]
    #elif TIM2_REMAP == 1
        #define TIM2_CH1_Gpio portA[15]
        #define TIM2_CH2_Gpio portB[3]
        #define TIM2_CH3_Gpio portA[2]
        #define TIM2_CH4_Gpio portA[3]
    #elif TIM2_REMAP == 2
        #define TIM2_CH1_Gpio portA[0]
        #define TIM2_CH2_Gpio portA[1]
        #define TIM2_CH3_Gpio portB[10]
        #define TIM2_CH4_Gpio portB[11]
    #elif TIM2_REMAP == 3
        #define TIM2_CH1_Gpio portA[15]
        #define TIM2_CH2_Gpio portB[3]
        #define TIM2_CH3_Gpio portB[10]
        #define TIM2_CH4_Gpio portB[11]
    #endif
#endif


#ifdef HAVE_TIM3
    #if TIM3_REMAP == 0
        #define TIM3_CH1_Gpio portA[6]
        #define TIM3_CH2_Gpio portA[7]
        #define TIM3_CH3_Gpio portB[0]
        #define TIM3_CH4_Gpio portB[1]
    #elif TIM3_REMAP == 1
        #error TIM3 remap 1 is preserved
    #elif TIM3_REMAP == 2
        #define TIM3_CH1_Gpio portB[4]
        #define TIM3_CH2_Gpio portB[5]
        #define TIM3_CH3_Gpio portB[0]
        #define TIM3_CH4_Gpio portB[1]
    #elif TIM3_REMAP == 3
        #error TIM3 remap 3 is imcomplete
    #endif
#endif

#ifdef HAVE_TIM4
    #if TIM4_REMAP == 0
        #define TIM4_CH1_Gpio portB[6]
        #define TIM4_CH2_Gpio portB[7]
        #define TIM4_CH3_Gpio portB[8]
        #define TIM4_CH4_Gpio portB[9]
    #elif TIM4_REMAP == 1
        #define TIM4_CH1_Gpio portD[12]
        #define TIM4_CH2_Gpio portD[13]
        #define TIM4_CH3_Gpio portD[14]
        #define TIM4_CH4_Gpio portD[15]
    #endif
#endif


#ifdef HAVE_TIM5
    #if TIM5_REMAP == 0
        #define TIM5_CH1_Gpio portA[0]
        #define TIM5_CH2_Gpio portA[1]
        #define TIM5_CH3_Gpio portA[2]
        #define TIM5_CH4_Gpio portA[3]
    #endif
#endif

#ifdef HAVE_TIM6
    #if TIM6_REMAP == 0
        #define TIM6_CH1_Gpio portA[0]
        #define TIM6_CH2_Gpio portA[1]
        #define TIM6_CH3_Gpio portA[2]
        #define TIM6_CH4_Gpio portA[3]
    #endif
#endif

#ifdef HAVE_TIM7
    #if TIM7_REMAP == 0
        #define TIM7_CH1_Gpio portA[0]
        #define TIM7_CH2_Gpio portA[1]
        #define TIM7_CH3_Gpio portA[2]
        #define TIM7_CH4_Gpio portA[3]
    #endif
#endif

#ifdef HAVE_TIM8
    #if TIM8_REMAP == 0
        #define TIM8_CH1_Gpio portC[6]
        #define TIM8_CH1N_Gpio portA[7]

        #define TIM8_CH2_Gpio portC[7]
        #define TIM8_CH2N_Gpio portB[0]

        #define TIM8_CH3_Gpio portC[8]
        #define TIM8_CH3N_Gpio portB[1]

        #define TIM8_CH4_Gpio portC[9]
    #elif TIM8_REMAP == 1
        #define TIM8_CH1_Gpio portB[6]
        #define TIM8_CH1N_Gpio portA[13]

        #define TIM8_CH2_Gpio portB[7]
        #define TIM8_CH2N_Gpio portA[14]

        #define TIM8_CH3_Gpio portB[8]
        #define TIM8_CH3N_Gpio portA[15]

        #define TIM8_CH4_Gpio portC[13]
    #endif
#endif


#ifdef HAVE_CAN1
    #define CAN1_RM0_TX_Gpio portA[12]
    #define CAN1_RM0_RX_Gpio portA[11]

    #define CAN1_RM1_TX_Gpio portB[9]
    #define CAN1_RM1_RX_Gpio portB[8]

    #define CAN1_RM3_TX_Gpio portD[1]
    #define CAN1_RM3_RX_Gpio portD[0]
    
    #if CAN1_REMAP == 0
        #define CAN1_TX_Gpio CAN1_RM0_TX_Gpio
        #define CAN1_RX_Gpio CAN1_RM0_RX_Gpio

    #elif CAN1_REMAP == 1
        #define CAN1_TX_Gpio CAN1_RM1_TX_Gpio
        #define CAN1_RX_Gpio CAN1_RM1_RX_Gpio

    #elif CAN1_REMAP == 2
        #warn no can1 remap2

    #elif CAN1_REMAP == 3
        #define CAN1_TX_Gpio CAN1_RM3_TX_Gpio
        #define CAN1_RX_Gpio CAN1_RM3_RX_Gpio

    #endif
#endif

#ifdef HAVE_OPA2
    #define OPA2_N0_Gpio portB[10]
    #define OPA2_P0_Gpio portB[14]
    #define OPA2_O0_Gpio portA[2]

    #define OPA2_N1_Gpio portA[5]
    #define OPA2_P1_Gpio portA[7]
    #define OPA2_O1_Gpio portA[4]

#endif

#ifdef HAVE_OPA1
    #define OPA1_N0_Gpio portB[11]
    #define OPA1_P0_Gpio portB[15]
    #define OPA1_O0_Gpio portA[3]

    #define OPA1_N1_Gpio portA[6]
    #define OPA1_P1_Gpio portB[0]
    #define OPA1_O1_Gpio portB[1]
#endif


#endif