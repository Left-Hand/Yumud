#ifndef __PERIPH_DEFS_H__

#define __PERIPH_DEFS_H__

#include "user_defs.h"

#ifdef CH32V203
    #define HAVE_GPIOA
    #define HAVE_GPIOB
    #define HAVE_GPIOC
    #define HAVE_GPIOD
#elif defined(CH32V303)

#elif defined(CH32V305)

#elif defined(CH32V307)

#endif

#define ENABLE 1
#define DISABLE 0

#define UART1_REMAP_ENABLE ENABLE
#define UART2_REMAP_ENABLE DISABLE

#ifdef HAVE_UART1
    #define UART1_TX_DMA_CH dma1Ch4
    #define UART1_RX_DMA_CH dma1Ch5

    #ifdef UART1_REMAP_ENABLE
    #define UART1_TX_Port portB
    #define UART1_TX_Pin GPIO_Pin_6

    #define UART1_RX_Port portB
    #define UART1_RX_Pin GPIO_Pin_7

    #define UART1_IT_PP 0
    #define UART1_IT_SP 1
    #define UART1_REMAP GPIO_Remap_USART1

    #else
    #define UART1_TX_Port GPIOA
    #define UART1_TX_Pin GPIO_Pin_9

    #define UART1_RX_Port GPIOA
    #define UART1_RX_Pin GPIO_Pin_10

    #define UART1_IT_PP 1
    #define UART1_IT_SP 0
    #define UART1_REMAP GPIO_Remap_USART1
    #endif
#endif


#ifdef HAVE_UART2
    #define UART2_TX_DMA_CH dma1Ch7
    #define UART2_RX_DMA_CH dma1Ch6

    #ifdef UART1_REMAP_ENABLE
    #define UART2_TX_Port portA
    #define UART2_TX_Pin GPIO_Pin_2

    #define UART2_RX_Port portA
    #define UART2_RX_Pin GPIO_Pin_3

    #define UART2_IT_PP 1
    #define UART2_IT_SP 1
    #define UART2_REMAP GPIO_Remap_USART2
    #else

    #define UART2_TX_Port portD
    #define UART2_TX_Pin GPIO_Pin_5

    #define UART2_RX_Port portD
    #define UART2_RX_Pin GPIO_Pin_6

    #define UART2_IT_PP 1
    #define UART2_IT_SP 1
    #define UART2_REMAP GPIO_Remap_USART2
    #endif
#endif


#ifdef HAVE_SPI1
    #define SPI1_REMAP_ENABLE ENABLE

    #if SPI1_REMAP_ENABLE == ENABLE
        #define SPI1_CS_Port portA
        #define SPI1_CS_Pin GPIO_Pin_15

        #define SPI1_Port portB
        #define SPI1_SCLK_Port portB
        #define SPI1_SCLK_Pin GPIO_Pin_3
        #define SPI1_MISO_Port portB
        #define SPI1_MISO_Pin GPIO_Pin_4
        #define SPI1_MOSI_Port portB
        #define SPI1_MOSI_Pin GPIO_Pin_5
        #define SPI1_REMAP GPIO_Remap_SPI1
    #else
        #define SPI1_CS_Port GPIOA
        #define SPI1_CS_Pin GPIO_Pin_4

        #define SPI1_Port GPIOA
        #define SPI1_SCLK_Port GPIOA
        #define SPI1_SCLK_Pin GPIO_Pin_5
        #define SPI1_MISO_Port GPIOA
        #define SPI1_MISO_Pin GPIO_Pin_6
        #define SPI1_MOSI_Port GPIOA
        #define SPI1_MOSI_Pin GPIO_Pin_7
        #define SPI1_REMAP GPIO_Remap_SPI1
    #endif
#endif


#ifdef HAVE_SPI2
    #define SPI2_CS_Port portB
    #define SPI2_CS_Pin GPIO_Pin_12

    #define SPI2_Port portB
    #define SPI2_SCLK_Port portB
    #define SPI2_SCLK_Pin GPIO_Pin_13
    #define SPI2_MISO_Port portB
    #define SPI2_MISO_Pin GPIO_Pin_14
    #define SPI2_MOSI_Port portB
    #define SPI2_MOSI_Pin GPIO_Pin_15
#endif



#ifdef HAVE_I2C1
#if (I2C1_REMAP_ENABLE == ENABLE)
    #define I2C1_Port GPIOB
    #define I2C1_SCL_Pin GPIO_Pin_8
    #define I2C1_SDA_Pin GPIO_Pin_9
    #define I2C1_REMAP GPIO_Remap_I2C1
#else
    #define I2C1_Port portB
    #define I2C1_SCL_Pin GPIO_Pin_6
    #define I2C1_SDA_Pin GPIO_Pin_7
    #define I2C1_REMAP GPIO_Remap_I2C1
#endif
#endif


#ifdef HAVE_I2C2
    #define I2C2_Port GPIOB
    #define I2C2_SCL_Pin GPIO_Pin_10
    #define I2C2_SDA_Pin GPIO_Pin_11
#endif

#ifdef HAVE_TIM1
    #define TIM1_CH1_Port portA
    #define TIM1_CH1_Pin GPIO_Pin_8
    #define TIM1_CH1N_Port portB
    #define TIM1_CH1N_Pin GPIO_Pin_13

    #define TIM1_CH2_Port portA
    #define TIM1_CH2_Pin GPIO_Pin_9
    #define TIM1_CH2N_Port portB
    #define TIM1_CH2N_Pin GPIO_Pin_14

    #define TIM1_CH3_Port portA
    #define TIM1_CH3_Pin GPIO_Pin_10
    #define TIM1_CH3N_Port portB
    #define TIM1_CH3N_Pin GPIO_Pin_15

    #define TIM1_CH4_Port portA
    #define TIM1_CH4_Pin GPIO_Pin_11
#endif


#ifdef HAVE_TIM2
    #if ((TIM2_REMAP == GPIO_FullRemap_TIM2) || (TIM2_REMAP == GPIO_PartialRemap1_TIM2))
        #define TIM2_CH1_Port portA
        #define TIM2_CH1_Pin GPIO_Pin_15

        #define TIM2_CH2_Port portB
        #define TIM2_CH2_Pin GPIO_Pin_3

    #else
        #define TIM2_CH1_Port portA
        #define TIM2_CH1_Pin GPIO_Pin_0

        #define TIM2_CH2_Port portA
        #define TIM2_CH2_Pin GPIO_Pin_1
    #endif

    #if ((TIM2_REMAP == GPIO_FullRemap_TIM2) || (TIM2_REMAP == GPIO_PartialRemap2_TIM2))
        #define TIM2_CH3_Port portB
        #define TIM2_CH3_Pin GPIO_Pin_10

        #define TIM2_CH4_Port portB
        #define TIM2_CH4_Pin GPIO_Pin_11

    #else
        #define TIM2_CH3_Port GPIOA
        #define TIM2_CH3_Pin GPIO_Pin_2

        #define TIM2_CH4_Port GPIOA
        #define TIM2_CH4_Pin GPIO_Pin_3

    #endif
#endif


#ifdef HAVE_TIM3
    #if TIM3_REMAP_ENABLE == ENABLE
    #define TIM3_CH1_Port portB
    #define TIM3_CH1_Pin GPIO_Pin_4

    #define TIM3_CH2_Port portB
    #define TIM3_CH2_Pin GPIO_Pin_5
    #else
    #define TIM3_CH1_Port portA
    #define TIM3_CH1_Pin GPIO_Pin_6

    #define TIM3_CH2_Port portA
    #define TIM3_CH2_Pin GPIO_Pin_7
    #endif
    #define TIM3_CH3_Port portB
    #define TIM3_CH3_Pin GPIO_Pin_0

    #define TIM3_CH4_Port portB
    #define TIM3_CH4_Pin GPIO_Pin_1

#endif

#ifdef HAVE_TIM4
    #define TIM4_CH1_Port portB
    #define TIM4_CH1_Pin GPIO_Pin_6

    #define TIM4_CH2_Port portB
    #define TIM4_CH2_Pin GPIO_Pin_7

    #define TIM4_CH3_Port portB
    #define TIM4_CH3_Pin GPIO_Pin_8

    #define TIM4_CH4_Port portB
    #define TIM4_CH4_Pin GPIO_Pin_9
#endif


#ifdef HAVE_CAN1
    #define CAN1_TX_RM0_Port GPIOA
    #define CAN1_RX_RM0_Port GPIOA
    #define CAN1_TX_RM0_Pin GPIO_Pin_12
    #define CAN1_RX_RM0_Pin GPIO_Pin_11

    #define CAN1_TX_RM1_Port GPIOB
    #define CAN1_RX_RM1_Port GPIOB
    #define CAN1_TX_RM1_Pin GPIO_Pin_9
    #define CAN1_RX_RM1_Pin GPIO_Pin_8

    #if CAN1_REMAP_ENABLE == ENABLE
        #define CAN1_Port GPIOB
        #define CAN1_TX_Port GPIOB
        #define CAN1_RX_Port GPIOB
        #define CAN1_TX_Pin GPIO_Pin_9
        #define CAN1_RX_Pin GPIO_Pin_8

        #define CAN1_REMAP GPIO_Remap1_CAN1
    #else
        #define CAN1_Port GPIOA
        #define CAN1_TX_Pin GPIO_Pin_12
        #define CAN1_RX_Pin GPIO_Pin_11
        #define CAN1_REMAP GPIO_Remap1_CAN1

    #endif
#endif

#ifdef HAVE_OPA2
    #define OPA2_O0_Pin GPIO_Pin_2
    #define OPA2_O1_Pin GPIO_Pin_4
    #define OPA2_N1_Pin GPIO_Pin_5
    #define OPA2_P1_Pin GPIO_Pin_7
    #define OPA2_O0_Port GPIOA
    #define OPA2_O1_Port GPIOA
    #define OPA2_N1_Port GPIOA
    #define OPA2_P1_Port GPIOA
    #define OPA2_P0_Pin GPIO_Pin_14
    #define OPA2_N0_Pin GPIO_Pin_10
    #define OPA2_N0_Port GPIOB
    #define OPA2_P0_Port GPIOB
#endif

#ifdef HAVE_OPA1
    #define OPA1_O0_Pin GPIO_Pin_3
    #define OPA1_N1_Pin GPIO_Pin_6
    #define OPA1_N1_Port GPIOA
    #define OPA1_O0_Port GPIOA
    #define OPA1_P1_Pin GPIO_Pin_0
    #define OPA1_O1_Pin GPIO_Pin_1
    #define OPA1_N0_Pin GPIO_Pin_11
    #define OPA1_P0_Pin GPIO_Pin_15
    #define OPA1_P1_Port GPIOB
    #define OPA1_O1_Port GPIOB
    #define OPA1_N0_Port GPIOB
    #define OPA1_P0_Port GPIOB
#endif

#endif