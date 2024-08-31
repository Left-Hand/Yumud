
#ifndef __N32G45X_IRQN_H__
#define __N32G45X_IRQN_H__

/** @addtogroup N32G45x_Library_Basic
 * @{
 */

#if !defined USE_STDPERIPH_DRIVER
/*
 * Comment the line below if you will not use the peripherals drivers.
   In this case, these drivers will not be included and the application code will
   be based on direct access to peripherals registers
   */
#define USE_STDPERIPH_DRIVER
#endif

/*
 * In the following line adjust the value of External High Speed oscillator (HSE)
   used in your application

   Tip: To avoid modifying this file each time you need to use different HSE, you
        can define the HSE value in your toolchain compiler preprocessor.
  */
#if !defined HSE_VALUE
#define HSE_VALUE (8000000) /*!< Value of the External oscillator in Hz */
#endif                      /* HSE_VALUE */

/*
 * In the following line adjust the External High Speed oscillator (HSE) Startup
   Timeout value
   */
#define HSE_STARTUP_TIMEOUT ((uint16_t)0x8000) /*!< Time out for HSE start up */

#define HSI_VALUE (8000000) /*!< Value of the Internal oscillator in Hz*/

#define __N32G45X_STDPERIPH_VERSION_MAIN (0x00) /*!< [31:24] main version */
#define __N32G45X_STDPERIPH_VERSION_SUB1 (0x04) /*!< [23:16] sub1 version */
#define __N32G45X_STDPERIPH_VERSION_SUB2 (0x00) /*!< [15:8]  sub2 version */
#define __N32G45X_STDPERIPH_VERSION_RC   (0x00) /*!< [7:0]  release candidate */

/**
 * @brief N32G45X Standard Peripheral Library version number
 */
#define __N32G45X_STDPERIPH_VERSION                                                                                    \
    ((__N32G45X_STDPERIPH_VERSION_MAIN << 24) | (__N32G45X_STDPERIPH_VERSION_SUB1 << 16)                               \
     | (__N32G45X_STDPERIPH_VERSION_SUB2 << 8) | (__N32G45X_STDPERIPH_VERSION_RC))

/*
 * Configuration of the Cortex-M4 Processor and Core Peripherals
 */
#ifdef N32G45X
#define __MPU_PRESENT 1          /*!< N32G45X devices does not provide an MPU */
#define __FPU_PRESENT 1          /*!< FPU present                            */
#endif                           /* N32G45X */
#define __NVIC_PRIO_BITS       4 /*!< N32G45X uses 4 Bits for the Priority Levels    */
#define __Vendor_SysTickConfig 0 /*!< Set to 1 if different SysTick Config is used */

/**
 * @brief N32G45X Interrupt Number Definition
 */
typedef enum IRQn
{
    /******  Cortex-M4 Processor Exceptions Numbers ***************************************************/
    NonMaskableInt_IRQn   = -14, /*!< 2 Non Maskable Interrupt                             */
    HardFault_IRQn        = -13, /*!< 3 Hard Fault Interrupt                               */
    MemoryManagement_IRQn = -12, /*!< 4 Cortex-M4 Memory Management Interrupt              */
    BusFault_IRQn         = -11, /*!< 5 Cortex-M4 Bus Fault Interrupt                      */
    UsageFault_IRQn       = -10, /*!< 6 Cortex-M4 Usage Fault Interrupt                    */
    SVCall_IRQn           = -5,  /*!< 11 Cortex-M4 SV Call Interrupt                       */
    DebugMonitor_IRQn     = -4,  /*!< 12 Cortex-M4 Debug Monitor Interrupt                 */
    PendSV_IRQn           = -2,  /*!< 14 Cortex-M4 Pend SV Interrupt                       */
    SysTick_IRQn          = -1,  /*!< 15 Cortex-M4 System Tick Interrupt                   */

    /******  N32G45X specific Interrupt Numbers ********************************************************/
    WWDG_IRQn          = 0,  /*!< Window WatchDog Interrupt                            */
    PVD_IRQn           = 1,  /*!< PVD through EXTI Line detection Interrupt            */
    TAMPER_IRQn        = 2,  /*!< Tamper Interrupt                                     */
    RTC_IRQn           = 3,  /*!< RTC global Interrupt                                 */
    FLASH_IRQn         = 4,  /*!< FLASH global Interrupt                               */
    RCC_IRQn           = 5,  /*!< RCC global Interrupt                                 */
    EXTI0_IRQn         = 6,  /*!< EXTI Line0 Interrupt                                 */
    EXTI1_IRQn         = 7,  /*!< EXTI Line1 Interrupt                                 */
    EXTI2_IRQn         = 8,  /*!< EXTI Line2 Interrupt                                 */
    EXTI3_IRQn         = 9,  /*!< EXTI Line3 Interrupt                                 */
    EXTI4_IRQn         = 10, /*!< EXTI Line4 Interrupt                                 */
    DMA1_Channel1_IRQn = 11, /*!< DMA1 Channel 1 global Interrupt                      */
    DMA1_Channel2_IRQn = 12, /*!< DMA1 Channel 2 global Interrupt                      */
    DMA1_Channel3_IRQn = 13, /*!< DMA1 Channel 3 global Interrupt                      */
    DMA1_Channel4_IRQn = 14, /*!< DMA1 Channel 4 global Interrupt                      */
    DMA1_Channel5_IRQn = 15, /*!< DMA1 Channel 5 global Interrupt                      */
    DMA1_Channel6_IRQn = 16, /*!< DMA1 Channel 6 global Interrupt                      */
    DMA1_Channel7_IRQn = 17, /*!< DMA1 Channel 7 global Interrupt                      */

    ADC1_2_IRQn          = 18, /*!< ADC1 and ADC2 global Interrupt                       */
    USB_HP_CAN1_TX_IRQn  = 19, /*!< USB Device High Priority or CAN1 TX Interrupts       */
    USB_LP_CAN1_RX0_IRQn = 20, /*!< USB Device Low Priority or CAN1 RX0 Interrupts       */
    CAN1_RX1_IRQn        = 21, /*!< CAN1 RX1 Interrupt                                   */
    CAN1_SCE_IRQn        = 22, /*!< CAN1 SCE Interrupt                                   */
    EXTI9_5_IRQn         = 23, /*!< External Line[9:5] Interrupts                        */
    TIM1_BRK_IRQn        = 24, /*!< TIM1 Break Interrupt                                 */
    TIM1_UP_IRQn         = 25, /*!< TIM1 Update Interrupt                                */
    TIM1_TRG_COM_IRQn    = 26, /*!< TIM1 Trigger and Commutation Interrupt               */
    TIM1_CC_IRQn         = 27, /*!< TIM1 Capture Compare Interrupt                       */
    TIM2_IRQn            = 28, /*!< TIM2 global Interrupt                                */
    TIM3_IRQn            = 29, /*!< TIM3 global Interrupt                                */
    TIM4_IRQn            = 30, /*!< TIM4 global Interrupt                                */
    I2C1_EV_IRQn         = 31, /*!< I2C1 Event Interrupt                                 */
    I2C1_ER_IRQn         = 32, /*!< I2C1 Error Interrupt                                 */
    I2C2_EV_IRQn         = 33, /*!< I2C2 Event Interrupt                                 */
    I2C2_ER_IRQn         = 34, /*!< I2C2 Error Interrupt                                 */
    SPI1_IRQn            = 35, /*!< SPI1 global Interrupt                                */
    SPI2_IRQn            = 36, /*!< SPI2 global Interrupt                                */
    USART1_IRQn          = 37, /*!< USART1 global Interrupt                              */
    USART2_IRQn          = 38, /*!< USART2 global Interrupt                              */
    USART3_IRQn          = 39, /*!< USART3 global Interrupt                              */
    EXTI15_10_IRQn       = 40, /*!< External Line[15:10] Interrupts                      */
    RTCAlarm_IRQn        = 41, /*!< RTC Alarm through EXTI Line Interrupt                */
    USBWakeUp_IRQn       = 42, /*!< USB Device WakeUp from suspend through EXTI Line Interrupt */
    TIM8_BRK_IRQn        = 43, /*!< TIM8 Break Interrupt                                 */
    TIM8_UP_IRQn         = 44, /*!< TIM8 Update Interrupt                                */
    TIM8_TRG_COM_IRQn    = 45, /*!< TIM8 Trigger and Commutation Interrupt               */
    TIM8_CC_IRQn         = 46, /*!< TIM8 Capture Compare Interrupt                       */
    ADC3_4_IRQn          = 47, /*!< ADC3 and ADC4 global Interrupt                       */
    RESERVE48_IRQn       = 48, /*!< RESERVE                                              */
    SDIO_IRQn            = 49, /*!< SDIO global Interrupt                                */
    TIM5_IRQn            = 50, /*!< TIM5 global Interrupt                                */
    SPI3_IRQn            = 51, /*!< SPI3 global Interrupt                                */
    UART4_IRQn           = 52, /*!< UART4 global Interrupt                               */
    UART5_IRQn           = 53, /*!< UART5 global Interrupt                               */
    TIM6_IRQn            = 54, /*!< TIM6 global Interrupt                                */
    TIM7_IRQn            = 55, /*!< TIM7 global Interrupt                                */
    DMA2_Channel1_IRQn   = 56, /*!< DMA2 Channel 1 global Interrupt                      */
    DMA2_Channel2_IRQn   = 57, /*!< DMA2 Channel 2 global Interrupt                      */
    DMA2_Channel3_IRQn   = 58, /*!< DMA2 Channel 3 global Interrupt                      */
    DMA2_Channel4_IRQn   = 59, /*!< DMA2 Channel 4 global Interrupt                      */
    DMA2_Channel5_IRQn   = 60, /*!< DMA2 Channel 5 global Interrupt                      */
    ETH_IRQn             = 61, /*!< Ethernet global Interrupt                            */
    ETH_WKUP_IRQn        = 62, /*!< Ethernet Wakeup through EXTI Line interrupt          */
    CAN2_TX_IRQn         = 63, /*!< CAN2 TX Interrupt                                    */
    CAN2_RX0_IRQn        = 64, /*!< CAN2 RX0 Interrupt                                   */
    CAN2_RX1_IRQn        = 65, /*!< CAN2 RX1 Interrupt                                   */
    CAN2_SCE_IRQn        = 66, /*!< CAN2 SCE Interrupt                                   */
    QSPI_IRQn            = 67, /*!< QSPI global Interrupt                                */
    DMA2_Channel6_IRQn   = 68, /*!< DMA2 Channel 6 global Interrupt                      */
    DMA2_Channel7_IRQn   = 69, /*!< DMA2 Channel 7 global Interrupt                      */
    I2C3_EV_IRQn         = 70, /*!< I2C3 Event Interrupt                                 */
    I2C3_ER_IRQn         = 71, /*!< I2C3 Error Interrupt                                 */
    I2C4_EV_IRQn         = 72, /*!< I2C4 Event Interrupt                                 */
    I2C4_ER_IRQn         = 73, /*!< I2C4 Error Interrupt                                 */
    UART6_IRQn           = 74, /*!< UART6 global Interrupt                               */
    UART7_IRQn           = 75, /*!< UART7 global Interrupt                               */
    DMA1_Channel8_IRQn   = 76, /*!< DMA1 Channel 8 global Interrupt                      */
    DMA2_Channel8_IRQn   = 77, /*!< DMA2 Channel 8 global Interrupt                      */
    DVP_IRQn             = 78, /*!< DVP global Interrupt                                 */
    SAC_IRQn             = 79, /*!< SAC global Interrupt                                 */
    MMU_IRQn             = 80, /*!< MMU global Interrupt                                 */
    TSC_IRQn             = 81, /*!< TSC global Interrupt                                 */
    COMP_1_2_3_IRQn      = 82, /*!< COMP1 & COMP2 & COMP3 global Interrupt               */
    COMP_4_5_6_IRQn      = 83, /*!< COMP4 & COMP5 & COMP6 global Interrupt               */
    COMP7_IRQn           = 84  /*!< COMP7 global Interrupt                               */

} IRQn_Type;

#endif