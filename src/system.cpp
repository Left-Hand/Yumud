#include "system.hpp"
#include "src/crc/crc.hpp"

real_t Sys::t;

static void PortC_Init( void ){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
    #ifdef HAVE_GPIOD
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE );
    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
    #endif
    PWR_BackupAccessCmd( ENABLE );
    RCC_LSEConfig( RCC_LSE_OFF );
    BKP_TamperPinCmd(DISABLE);
    PWR_BackupAccessCmd(DISABLE);
}


void Sys::Misc::prework(){
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    Systick_Init();
    PortC_Init();
}


void Sys::Misc::reset(){
    NVIC_SystemReset();
}


void Sys::Clock::reCalculateTime(){
        #ifdef USE_IQ
        t.value = msTick * (int)(0.001 * (1 << GLOBAL_Q));
        #else
        t = msTick * (1 / 1000.0f);
        #endif
    }


real_t Sys::Clock::getCurrentSeconds(){
        // reCalculateTime();
        return t;
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


static RCC_ClocksTypeDef RCC_CLK;
static void ClockUpdate(){
	RCC_GetClocksFreq(&RCC_CLK);//Get chip frequencies
}

uint32_t Sys::Clock::getSystemFreq(){
    ClockUpdate();
    return RCC_CLK.SYSCLK_Frequency;
}


uint32_t Sys::Clock::getAPB1Freq(){
    ClockUpdate();
    return RCC_CLK.PCLK1_Frequency;
}


void Sys::Clock::setAHBDiv(const uint8_t & _div){
    uint8_t div = NEXT_POWER_OF_2(_div);
    switch(div){
        case 1:
            RCC_HCLKConfig(RCC_SYSCLK_Div1);
            break;
        case 2:
            RCC_HCLKConfig(RCC_SYSCLK_Div2);
            break;
        default:return;
    }
}

void Sys::Clock::setAPB1Div(const uint8_t & _div){
    uint8_t div = NEXT_POWER_OF_2(_div);
    switch(div){
        case 1:
            RCC_PCLK1Config(RCC_HCLK_Div1);
            break;
        case 2:
            RCC_PCLK1Config(RCC_HCLK_Div2);
            break;
        case 4:
            RCC_PCLK1Config(RCC_HCLK_Div4);
            break;
        case 8:
            RCC_PCLK1Config(RCC_HCLK_Div8);
            break;
        case 16:
            RCC_PCLK1Config(RCC_HCLK_Div16);
            break;
    }
}

void Sys::Clock::setAPB2Div(const uint8_t & _div){
    uint8_t div = NEXT_POWER_OF_2(_div);
    switch(div){
        case 1:
            RCC_PCLK2Config(RCC_HCLK_Div1);
            break;
        case 2:
            RCC_PCLK2Config(RCC_HCLK_Div2);
            break;
        case 4:
            RCC_PCLK2Config(RCC_HCLK_Div4);
            break;
        case 8:
            RCC_PCLK2Config(RCC_HCLK_Div8);
            break;
        case 16:
            RCC_PCLK2Config(RCC_HCLK_Div16);
            break;
    }
}


uint32_t Sys::Clock::getAPB2Freq(){
    ClockUpdate();
    return RCC_CLK.PCLK2_Frequency;
}

uint32_t Sys::Clock::getAHBFreq(){
    ClockUpdate();
    return RCC_CLK.HCLK_Frequency;
}

void Sys::Clock::setAHBFreq(const uint32_t & freq){
    setAHBDiv(getAHBFreq() / freq);
}

void Sys::Clock::setAPB1Freq(const uint32_t & freq){
    setAPB1Div(getAPB1Freq() / freq);
}

void Sys::Clock::setAPB2Freq(const uint32_t & freq){
    setAPB2Div(getAPB2Freq() / freq);
}