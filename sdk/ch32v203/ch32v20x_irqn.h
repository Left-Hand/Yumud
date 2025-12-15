#pragma once

#ifdef __cplusplus
 extern "C" {
#endif 
/* Interrupt Number Definition, according to the selected device */
typedef enum IRQn
{
    /******  RISC-V Processor Exceptions Numbers *******************************************************/
    NonMaskableInt_IRQn = 2, /* 2 Non Maskable Interrupt                             */
    EXC_IRQn = 3,            /* 3 Exception Interrupt                                */
    Ecall_M_Mode_IRQn = 5,   /* 5 Ecall M Mode Interrupt                             */
    Ecall_U_Mode_IRQn = 8,   /* 8 Ecall U Mode Interrupt                             */
    Break_Point_IRQn = 9,    /* 9 Break Point Interrupt                              */
    SysTicK_IRQn = 12,       /* 12 System timer Interrupt                            */
    Software_IRQn = 14,      /* 14 software Interrupt                                */

    /******  RISC-V specific Interrupt Numbers *********************************************************/
    WWDG_IRQn = 16,            /* Window WatchDog Interrupt                            */
    PVD_IRQn = 17,             /* PVD through EXTI Line detection Interrupt            */
    TAMPER_IRQn = 18,          /* Tamper Interrupt                                     */
    RTC_IRQn = 19,             /* RTC global Interrupt                                 */
    FLASH_IRQn = 20,           /* FLASH global Interrupt                               */
    RCC_IRQn = 21,             /* RCC global Interrupt                                 */
    EXTI0_IRQn = 22,           /* EXTI Line0 Interrupt                                 */
    EXTI1_IRQn = 23,           /* EXTI Line1 Interrupt                                 */
    EXTI2_IRQn = 24,           /* EXTI Line2 Interrupt                                 */
    EXTI3_IRQn = 25,           /* EXTI Line3 Interrupt                                 */
    EXTI4_IRQn = 26,           /* EXTI Line4 Interrupt                                 */
    DMA1_Channel1_IRQn = 27,   /* DMA1 Channel 1 global Interrupt                      */
    DMA1_Channel2_IRQn = 28,   /* DMA1 Channel 2 global Interrupt                      */
    DMA1_Channel3_IRQn = 29,   /* DMA1 Channel 3 global Interrupt                      */
    DMA1_Channel4_IRQn = 30,   /* DMA1 Channel 4 global Interrupt                      */
    DMA1_Channel5_IRQn = 31,   /* DMA1 Channel 5 global Interrupt                      */
    DMA1_Channel6_IRQn = 32,   /* DMA1 Channel 6 global Interrupt                      */
    DMA1_Channel7_IRQn = 33,   /* DMA1 Channel 7 global Interrupt                      */
    ADC_IRQn = 34,             /* ADC1 and ADC2 global Interrupt                       */
    USB_HP_CAN1_TX_IRQn = 35,  /* USB Device High Priority or CAN1 TX Interrupts       */
    USB_LP_CAN1_RX0_IRQn = 36, /* USB Device Low Priority or CAN1 RX0 Interrupts       */
    CAN1_RX1_IRQn = 37,        /* CAN1 RX1 Interrupt                                   */
    CAN1_SCE_IRQn = 38,        /* CAN1 SCE Interrupt                                   */
    EXTI9_5_IRQn = 39,         /* External Line[9:5] Interrupts                        */
    TIM1_BRK_IRQn = 40,        /* TIM1 Break Interrupt                                 */
    TIM1_UP_IRQn = 41,         /* TIM1 Update Interrupt                                */
    TIM1_TRG_COM_IRQn = 42,    /* TIM1 Trigger and Commutation Interrupt               */
    TIM1_CC_IRQn = 43,         /* TIM1 Capture Compare Interrupt                       */
    TIM2_IRQn = 44,            /* TIM2 global Interrupt                                */
    TIM3_IRQn = 45,            /* TIM3 global Interrupt                                */
    TIM4_IRQn = 46,            /* TIM4 global Interrupt                                */
    I2C1_EV_IRQn = 47,         /* I2C1 Event Interrupt                                 */
    I2C1_ER_IRQn = 48,         /* I2C1 Error Interrupt                                 */
    I2C2_EV_IRQn = 49,         /* I2C2 Event Interrupt                                 */
    I2C2_ER_IRQn = 50,         /* I2C2 Error Interrupt                                 */
    SPI1_IRQn = 51,            /* SPI1 global Interrupt                                */
    SPI2_IRQn = 52,            /* SPI2 global Interrupt                                */
    USART1_IRQn = 53,          /* USART1 global Interrupt                              */
    USART2_IRQn = 54,          /* USART2 global Interrupt                              */
    USART3_IRQn = 55,          /* USART3 global Interrupt                              */
    EXTI15_10_IRQn = 56,       /* External Line[15:10] Interrupts                      */
    RTCAlarm_IRQn = 57,        /* RTC Alarm through EXTI Line Interrupt                */
    USBWakeUp_IRQn = 58,       /* USB Device WakeUp from suspend through EXTI Line Interrupt 	*/
    USBHD_IRQn = 59,           /* USBHD global Interrupt                               */
    USBHDWakeUp_IRQn = 60,     /* USB Host/Device WakeUp Interrupt                     */

#ifdef CH32V20x_D6
    UART4_IRQn = 61,         /* UART4 global Interrupt                               */
    DMA1_Channel8_IRQn = 62, /* DMA1 Channel 8 global Interrupt                      */

#elif defined(CH32V20x_D8)
    ETH_IRQn = 61,           /* ETH global Interrupt               	                 */
    ETHWakeUp_IRQn = 62,     /* ETH WakeUp Interrupt                       			 */
    TIM5_IRQn = 65,          /* TIM5 global Interrupt                                */
    UART4_IRQn = 66,         /* UART4 global Interrupt                               */
    DMA1_Channel8_IRQn = 67, /* DMA1 Channel 8 global Interrupt                      */
    OSC32KCal_IRQn = 68,     /* OSC32K global Interrupt                              */
    OSCWakeUp_IRQn = 69,     /* OSC32K WakeUp Interrupt                              */

#elif defined(CH32V20x_D8W)
    ETH_IRQn = 61,           /* ETH global Interrupt               	                 */
    ETHWakeUp_IRQn = 62,     /* ETH WakeUp Interrupt                       			 */
    BB_IRQn = 63,            /* BLE BB global Interrupt                              */
    LLE_IRQn = 64,           /* BLE LLE global Interrupt                             */
    TIM5_IRQn = 65,          /* TIM5 global Interrupt                                */
    UART4_IRQn = 66,         /* UART4 global Interrupt                               */
    DMA1_Channel8_IRQn = 67, /* DMA1 Channel 8 global Interrupt                      */
    OSC32KCal_IRQn = 68,     /* OSC32K global Interrupt                              */
    OSCWakeUp_IRQn = 69,     /* OSC32K WakeUp Interrupt                              */
#endif

} IRQn_Type;

#define HardFault_IRQn    EXC_IRQn
#define ADC1_2_IRQn       ADC_IRQn


#ifdef __cplusplus
}
#endif 