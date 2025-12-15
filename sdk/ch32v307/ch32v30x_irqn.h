#pragma once

#ifdef __cplusplus
 extern "C" {
#endif 

/* Interrupt Number Definition, according to the selected device */	 
typedef enum IRQn
{
 /******  RISC-V Processor Exceptions Numbers *******************************************************/
  NonMaskableInt_IRQn         = 2,       /* 2 Non Maskable Interrupt                             */
  EXC_IRQn                    = 3,       /* 3 Exception Interrupt                                */
  Ecall_M_Mode_IRQn           = 5,       /* 5 Ecall M Mode Interrupt                             */
  Ecall_U_Mode_IRQn           = 8,       /* 8 Ecall U Mode Interrupt                             */
  Break_Point_IRQn            = 9,       /* 9 Break Point Interrupt                              */
  SysTicK_IRQn                = 12,      /* 12 System timer Interrupt                            */
  Software_IRQn               = 14,      /* 14 software Interrupt                                */

 /******  RISC-V specific Interrupt Numbers *********************************************************/
  WWDG_IRQn                   = 16,      /* Window WatchDog Interrupt                            */
  PVD_IRQn                    = 17,      /* PVD through EXTI Line detection Interrupt            */
  TAMPER_IRQn                 = 18,      /* Tamper Interrupt                                     */
  RTC_IRQn                    = 19,      /* RTC global Interrupt                                 */
  FLASH_IRQn                  = 20,      /* FLASH global Interrupt                               */
  RCC_IRQn                    = 21,      /* RCC global Interrupt                                 */
  EXTI0_IRQn                  = 22,      /* EXTI Line0 Interrupt                                 */
  EXTI1_IRQn                  = 23,      /* EXTI Line1 Interrupt                                 */
  EXTI2_IRQn                  = 24,      /* EXTI Line2 Interrupt                                 */
  EXTI3_IRQn                  = 25,      /* EXTI Line3 Interrupt                                 */
  EXTI4_IRQn                  = 26,      /* EXTI Line4 Interrupt                                 */
  DMA1_Channel1_IRQn          = 27,      /* DMA1 Channel 1 global Interrupt                      */
  DMA1_Channel2_IRQn          = 28,      /* DMA1 Channel 2 global Interrupt                      */
  DMA1_Channel3_IRQn          = 29,      /* DMA1 Channel 3 global Interrupt                      */
  DMA1_Channel4_IRQn          = 30,      /* DMA1 Channel 4 global Interrupt                      */
  DMA1_Channel5_IRQn          = 31,      /* DMA1 Channel 5 global Interrupt                      */
  DMA1_Channel6_IRQn          = 32,      /* DMA1 Channel 6 global Interrupt                      */
  DMA1_Channel7_IRQn          = 33,      /* DMA1 Channel 7 global Interrupt                      */
  ADC_IRQn                    = 34,      /* ADC1 and ADC2 global Interrupt                       */
  USB_HP_CAN1_TX_IRQn         = 35,      /* USB Device High Priority or CAN1 TX Interrupts       */
  USB_LP_CAN1_RX0_IRQn        = 36,      /* USB Device Low Priority or CAN1 RX0 Interrupts       */
  CAN1_RX1_IRQn               = 37,      /* CAN1 RX1 Interrupt                                   */
  CAN1_SCE_IRQn               = 38,      /* CAN1 SCE Interrupt                                   */
  EXTI9_5_IRQn                = 39,      /* External Line[9:5] Interrupts                        */
  TIM1_BRK_IRQn               = 40,      /* TIM1 Break Interrupt                                 */
  TIM1_UP_IRQn                = 41,      /* TIM1 Update Interrupt                                */
  TIM1_TRG_COM_IRQn           = 42,      /* TIM1 Trigger and Commutation Interrupt               */
  TIM1_CC_IRQn                = 43,      /* TIM1 Capture Compare Interrupt                       */
  TIM2_IRQn                   = 44,      /* TIM2 global Interrupt                                */
  TIM3_IRQn                   = 45,      /* TIM3 global Interrupt                                */
  TIM4_IRQn                   = 46,      /* TIM4 global Interrupt                                */
  I2C1_EV_IRQn                = 47,      /* I2C1 Event Interrupt                                 */
  I2C1_ER_IRQn                = 48,      /* I2C1 Error Interrupt                                 */
  I2C2_EV_IRQn                = 49,      /* I2C2 Event Interrupt                                 */
  I2C2_ER_IRQn                = 50,      /* I2C2 Error Interrupt                                 */
  SPI1_IRQn                   = 51,      /* SPI1 global Interrupt                                */
  SPI2_IRQn                   = 52,      /* SPI2 global Interrupt                                */
  USART1_IRQn                 = 53,      /* USART1 global Interrupt                              */
  USART2_IRQn                 = 54,      /* USART2 global Interrupt                              */
  USART3_IRQn                 = 55,      /* USART3 global Interrupt                              */
  EXTI15_10_IRQn              = 56,      /* External Line[15:10] Interrupts                      */
  RTCAlarm_IRQn               = 57,      /* RTC Alarm through EXTI Line Interrupt                */

#ifdef CH32V30x_D8
  TIM8_BRK_IRQn               = 59,      /* TIM8 Break Interrupt                                 */
  TIM8_UP_IRQn                = 60,      /* TIM8 Update Interrupt                                */
  TIM8_TRG_COM_IRQn           = 61,      /* TIM8 Trigger and Commutation Interrupt               */
  TIM8_CC_IRQn                = 62,      /* TIM8 Capture Compare Interrupt                       */
  RNG_IRQn                    = 63,      /* RNG global Interrupt                                 */
  FSMC_IRQn                   = 64,      /* FSMC global Interrupt                                */
  SDIO_IRQn                   = 65,      /* SDIO global Interrupt                                */
  TIM5_IRQn                   = 66,      /* TIM5 global Interrupt                                */
  SPI3_IRQn                   = 67,      /* SPI3 global Interrupt                                */
  UART4_IRQn                  = 68,      /* UART4 global Interrupt                               */
  UART5_IRQn                  = 69,      /* UART5 global Interrupt                               */
  TIM6_IRQn                   = 70,      /* TIM6 global Interrupt                                */
  TIM7_IRQn                   = 71,      /* TIM7 global Interrupt                                */
  DMA2_Channel1_IRQn          = 72,      /* DMA2 Channel 1 global Interrupt                      */
  DMA2_Channel2_IRQn          = 73,      /* DMA2 Channel 2 global Interrupt                      */
  DMA2_Channel3_IRQn          = 74,      /* DMA2 Channel 3 global Interrupt                      */
  DMA2_Channel4_IRQn          = 75,      /* DMA2 Channel 4 global Interrupt                      */
  DMA2_Channel5_IRQn          = 76,      /* DMA2 Channel 5 global Interrupt                      */
  OTG_FS_IRQn                 = 83,      /* OTGFS global Interrupt                               */
  UART6_IRQn                  = 87,      /* UART6 global Interrupt                               */
  UART7_IRQn                  = 88,      /* UART7 global Interrupt                               */
  UART8_IRQn                  = 89,      /* UART8 global Interrupt                               */
  TIM9_BRK_IRQn               = 90,      /* TIM9 Break Interrupt                                 */
  TIM9_UP_IRQn                = 91,      /* TIM9 Update Interrupt                                */
  TIM9_TRG_COM_IRQn           = 92,      /* TIM9 Trigger and Commutation Interrupt               */
  TIM9_CC_IRQn                = 93,      /* TIM9 Capture Compare Interrupt                       */
  TIM10_BRK_IRQn              = 94,      /* TIM10 Break Interrupt                                */
  TIM10_UP_IRQn               = 95,      /* TIM10 Update Interrupt                               */
  TIM10_TRG_COM_IRQn          = 96,      /* TIM10 Trigger and Commutation Interrupt              */
  TIM10_CC_IRQn               = 97,      /* TIM10 Capture Compare Interrupt                      */
  DMA2_Channel6_IRQn          = 98,      /* DMA2 Channel 6 global Interrupt                      */
  DMA2_Channel7_IRQn          = 99,      /* DMA2 Channel 7 global Interrupt                      */
  DMA2_Channel8_IRQn          = 100,     /* DMA2 Channel 8 global Interrupt                      */
  DMA2_Channel9_IRQn          = 101,     /* DMA2 Channel 9 global Interrupt                      */
  DMA2_Channel10_IRQn         = 102,     /* DMA2 Channel 10 global Interrupt                     */
  DMA2_Channel11_IRQn         = 103,     /* DMA2 Channel 11 global Interrupt                     */

#elif defined  (CH32V30x_D8C)
  USBWakeUp_IRQn              = 58,      /* USB Device WakeUp from suspend through EXTI Line Interrupt */
  TIM8_BRK_IRQn               = 59,      /* TIM8 Break Interrupt                                 */
  TIM8_UP_IRQn                = 60,      /* TIM8 Update Interrupt                                */
  TIM8_TRG_COM_IRQn           = 61,      /* TIM8 Trigger and Commutation Interrupt               */
  TIM8_CC_IRQn                = 62,      /* TIM8 Capture Compare Interrupt                       */
  RNG_IRQn                    = 63,      /* RNG global Interrupt                                 */
  FSMC_IRQn                   = 64,      /* FSMC global Interrupt                                */
  SDIO_IRQn                   = 65,      /* SDIO global Interrupt                                */
  TIM5_IRQn                   = 66,      /* TIM5 global Interrupt                                */
  SPI3_IRQn                   = 67,      /* SPI3 global Interrupt                                */
  UART4_IRQn                  = 68,      /* UART4 global Interrupt                               */
  UART5_IRQn                  = 69,      /* UART5 global Interrupt                               */
  TIM6_IRQn                   = 70,      /* TIM6 global Interrupt                                */
  TIM7_IRQn                   = 71,      /* TIM7 global Interrupt                                */
  DMA2_Channel1_IRQn          = 72,      /* DMA2 Channel 1 global Interrupt                      */
  DMA2_Channel2_IRQn          = 73,      /* DMA2 Channel 2 global Interrupt                      */
  DMA2_Channel3_IRQn          = 74,      /* DMA2 Channel 3 global Interrupt                      */
  DMA2_Channel4_IRQn          = 75,      /* DMA2 Channel 4 global Interrupt                      */
  DMA2_Channel5_IRQn          = 76,      /* DMA2 Channel 5 global Interrupt                      */
  ETH_IRQn                    = 77,      /* ETH global Interrupt                                 */
  ETH_WKUP_IRQn               = 78,      /* ETH WakeUp Interrupt                                 */
  CAN2_TX_IRQn                = 79,      /* CAN2 TX Interrupts                                   */
  CAN2_RX0_IRQn               = 80,      /* CAN2 RX0 Interrupts                                  */
  CAN2_RX1_IRQn               = 81,      /* CAN2 RX1 Interrupt                                   */
  CAN2_SCE_IRQn               = 82,      /* CAN2 SCE Interrupt                                   */
  OTG_FS_IRQn                 = 83,      /* OTGFS global Interrupt                               */
  USBHSWakeup_IRQn            = 84,      /* USBHS WakeUp Interrupt                               */
  USBHS_IRQn                  = 85,      /* USBHS global Interrupt                               */
  DVP_IRQn                    = 86,      /* DVP global Interrupt                                 */
  UART6_IRQn                  = 87,      /* UART6 global Interrupt                               */
  UART7_IRQn                  = 88,      /* UART7 global Interrupt                               */
  UART8_IRQn                  = 89,      /* UART8 global Interrupt                               */
  TIM9_BRK_IRQn               = 90,      /* TIM9 Break Interrupt                                 */
  TIM9_UP_IRQn                = 91,      /* TIM9 Update Interrupt                                */
  TIM9_TRG_COM_IRQn           = 92,      /* TIM9 Trigger and Commutation Interrupt               */
  TIM9_CC_IRQn                = 93,      /* TIM9 Capture Compare Interrupt                       */
  TIM10_BRK_IRQn              = 94,      /* TIM10 Break Interrupt                                */
  TIM10_UP_IRQn               = 95,      /* TIM10 Update Interrupt                               */
  TIM10_TRG_COM_IRQn          = 96,      /* TIM10 Trigger and Commutation Interrupt              */
  TIM10_CC_IRQn               = 97,      /* TIM10 Capture Compare Interrupt                      */
  DMA2_Channel6_IRQn          = 98,      /* DMA2 Channel 6 global Interrupt                      */
  DMA2_Channel7_IRQn          = 99,      /* DMA2 Channel 7 global Interrupt                      */
  DMA2_Channel8_IRQn          = 100,     /* DMA2 Channel 8 global Interrupt                      */
  DMA2_Channel9_IRQn          = 101,     /* DMA2 Channel 9 global Interrupt                      */
  DMA2_Channel10_IRQn         = 102,     /* DMA2 Channel 10 global Interrupt                     */
  DMA2_Channel11_IRQn         = 103,     /* DMA2 Channel 11 global Interrupt                     */

#endif
} IRQn_Type;

#define HardFault_IRQn   EXC_IRQn
#define ADC1_2_IRQn      ADC_IRQn

#ifdef __cplusplus
}
#endif 