#include "system.hpp"

#include "../sys/core/platform.h"
#include "../hal/crc/crc.hpp"

#include "../sys/clock/clock.h"
#include "../sys/constants/enums.hpp"

#ifdef N32G45X
#define M_RCC_CONFIGER RCC_ConfigHclk
#else
#define M_RCC_CONFIGER RCC_HCLKConfig
#endif

#ifdef N32G45X
#define M_PCLK1_CONFIGER RCC_ConfigPclk1
#else
#define M_PCLK1_CONFIGER RCC_PCLK1Config
#endif

#ifdef N32G45X
#define M_PCLK2_CONFIGER RCC_ConfigPclk2
#else
#define M_PCLK2_CONFIGER RCC_PCLK2Config
#endif

#ifdef N32G45X
#define M_CLOCK_TYPEDEF RCC_ClocksType
#else
#define M_CLOCK_TYPEDEF RCC_ClocksTypeDef
#endif

#ifdef N32G45X
#define M_RCC_CLK_GETTER RCC_GetClocksFreqValue
#else
#define M_RCC_CLK_GETTER RCC_GetClocksFreq
#endif

void Sys::Clock::delayMs(const uint32_t ms){
    delay(ms);
}

void Sys::Clock::delayUs(const uint32_t us){
    delayMicroseconds(us);
}

void Sys::Misc::prework(){
    #ifdef N32G45X
    RCC_ConfigPclk1(RCC_HCLK_DIV1);
    RCC_ConfigPclk2(RCC_HCLK_DIV1);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    Systick_Init();

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOC, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_AFIO, ENABLE);

    #ifdef HAVE_GPIOD
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE );
    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
    #endif
    PWR_BackupAccessEnable(ENABLE);
    // RCC_LSEConfig(RCC_LSE_DISABLE);
    //TODO
    BKP_TPEnable(DISABLE);
    PWR_BackupAccessEnable(DISABLE);
    #else
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    Systick_Init();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    #ifdef HAVE_GPIOD
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    //invalid for ch32v307vct6
    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
    #endif

    #ifdef HAVE_GPIOE
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    #endif
    PWR_BackupAccessCmd( ENABLE );
    RCC_LSEConfig( RCC_LSE_OFF );
    BKP_TamperPinCmd(DISABLE);
    PWR_BackupAccessCmd(DISABLE);
    #endif
}


void Sys::Misc::reset(){
    __disable_irq();
    __disable_irq();
    NVIC_SystemReset();
}


void Sys::Clock::reCalculateTime(){
        #ifdef USE_IQ
        t.value = _iq((micros() * (1 << GLOBAL_Q)) / 1000000);
        #else
        t = msTick * (1 / 1000.0f);
        #endif
    }


uint64_t Sys::Chip::getChipId(){
    static uint32_t chip_id[2] = {
        *(volatile uint32_t *)0x1FFFF7E8,
        *(volatile uint32_t *)0x1FFFF7EC
    };
    return ((uint64_t)chip_id[1] << 32) | chip_id[0];
}

uint32_t Sys::Chip::getFlashSize(){
    static uint32_t chip_flash_size = operator"" _KB(*(volatile uint16_t *)0x1FFFF7E0);
    return chip_flash_size;
}

uint32_t Sys::Chip::getChipIdCrc(){
    static uint32_t chip_id_crc = 0;
    if(!chip_id_crc){
        crc.init();
        crc.clear();
        uint64_t chip_id = getChipId();
        chip_id_crc = crc.update({(uint32_t)chip_id, (uint32_t)(chip_id >> 32)});
    }
    return chip_id_crc;
}


static M_CLOCK_TYPEDEF RCC_CLK;

static void ClockUpdate(){
	M_RCC_CLK_GETTER(&RCC_CLK);//Get chip frequencies
}

#ifdef N32G45X
#define M_RCC_SYSCLK(inst) inst.SysclkFreq;
#define M_RCC_HCLK(inst) inst.HclkFreq;
#define M_RCC_PCLK1(inst) inst.Pclk1Freq;
#define M_RCC_PCLK2(inst) inst.Pclk2Freq;
#define M_RCC_ADCPLL(inst) inst.AdcPllClkFreq;
#define M_RCC_ADCHCLK(inst) inst.AdcHClkFreq;

#else
#define M_RCC_SYSCLK(inst) inst.SYSCLK_Frequency;
#define M_RCC_HCLK(inst) inst.HCLK_Frequency;
#define M_RCC_PCLK1(inst) inst.PCLK1_Frequency;
#define M_RCC_PCLK2(inst) inst.PCLK2_Frequency;
#define M_RCC_ADC_CLK(inst) inst.ADCCLK_Frequency;
#endif



uint32_t Sys::Clock::getSystemFreq(){
    ClockUpdate();
    return M_RCC_SYSCLK(RCC_CLK);
}


uint32_t Sys::Clock::getAPB1Freq(){
    ClockUpdate();
    return M_RCC_PCLK1(RCC_CLK);
}


void Sys::Clock::setAHBDiv(const uint8_t _div){

    uint8_t div = NEXT_POWER_OF_2(_div);
    switch(div){
        case 1:
            M_RCC_CONFIGER(RCC_SYSCLK_Div1);
            
            break;
        case 2:
            M_RCC_CONFIGER(RCC_SYSCLK_Div2);
            break;
        default:return;
    }

}

void Sys::Clock::setAPB1Div(const uint8_t _div){
    uint8_t div = NEXT_POWER_OF_2(_div);
    switch(div){
        case 1:
            M_PCLK1_CONFIGER(RCC_HCLK_Div1);
            break;
        case 2:
            M_PCLK1_CONFIGER(RCC_HCLK_Div2);
            break;
        case 4:
            M_PCLK1_CONFIGER(RCC_HCLK_Div4);
            break;
        case 8:
            M_PCLK1_CONFIGER(RCC_HCLK_Div8);
            break;
        case 16:
            M_PCLK1_CONFIGER(RCC_HCLK_Div16);
            break;
    }
}

void Sys::Clock::setAPB2Div(const uint8_t _div){

    uint8_t div = NEXT_POWER_OF_2(_div);
    switch(div){
        case 1:
            M_PCLK2_CONFIGER(RCC_HCLK_Div1);
            break;
        case 2:
            M_PCLK2_CONFIGER(RCC_HCLK_Div2);
            break;
        case 4:
            M_PCLK2_CONFIGER(RCC_HCLK_Div4);
            break;
        case 8:
            M_PCLK2_CONFIGER(RCC_HCLK_Div8);
            break;
        case 16:
            M_PCLK2_CONFIGER(RCC_HCLK_Div16);
            break;
    }
}


uint32_t Sys::Clock::getAPB2Freq(){
    ClockUpdate();
    return M_RCC_PCLK2(RCC_CLK);
}

uint32_t Sys::Clock::getAHBFreq(){
    ClockUpdate();
    return M_RCC_HCLK(RCC_CLK);
}

void Sys::Clock::setAHBFreq(const uint32_t freq){
    setAHBDiv(getAHBFreq() / freq);
}

void Sys::Clock::setAPB1Freq(const uint32_t freq){
    setAPB1Div(getAPB1Freq() / freq);
}

void Sys::Clock::setAPB2Freq(const uint32_t freq){
    setAPB2Div(getAPB2Freq() / freq);
}


bool Sys::Exception::isInterruptPending(){
    #ifdef ARCH_QKV4
    return QingKeV4::isInterruptPending();
    #else
    return false;
    #endif
}

bool Sys::Exception::isIntrruptActing(){
    #ifdef ARCH_QKV4
    return QingKeV4::isIntrruptActing();
    #else
    return false;
    #endif
}

uint8_t Sys::Exception::getInterruptDepth(){
    #ifdef ARCH_QKV4
    return QingKeV4::getInterruptDepth();
    #else
    return 0;
    #endif
}