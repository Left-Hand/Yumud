#include "system.hpp"

#include "core/platform.hpp"
#include "core/sdk.hpp"
#include "core/arch.hpp"

#include "hal/crc/crc.hpp"

#include "core/clock/clock.hpp"

#include "core/debug/debug.hpp"

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

using namespace ymd;

void sys::clock::delay_ms(const uint32_t ms){
    delay(ms);
}

void sys::clock::delay_us(const uint32_t us){
    udelay(us);
}

void sys::preinit(){
    #ifdef N32G45X
    RCC_ConfigPclk1(RCC_HCLK_DIV1);
    RCC_ConfigPclk2(RCC_HCLK_DIV1);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    systick_Init();

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOC, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_AFIO, ENABLE);

    #ifdef ENABLE_GPIOD
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE );
    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
    #endif
    PWR_BackupAccessEnable(ENABLE);
    // RCC_LSEConfig(RCC_LSE_DISABLE);
    //TODO
    BKP_TPEnable(DISABLE);
    PWR_BackupAccessEnable(DISABLE);
    #elif defined(CH32V20X)
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    Systick_Init();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    #ifdef ENABLE_GPIOD
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    //invalid for ch32v307vct6
    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
    #endif

    #ifdef ENABLE_GPIOE
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    #endif
    PWR_BackupAccessCmd( ENABLE );
    RCC_LSEConfig( RCC_LSE_OFF );
    BKP_TamperPinCmd(DISABLE);
    PWR_BackupAccessCmd(DISABLE);
    #elif defined(CH32V30X)
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    Systick_Init();

    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );
    PWR_BackupAccessCmd( ENABLE );
    RCC_LSEConfig( RCC_LSE_OFF );
    BKP_TamperPinCmd(DISABLE);
    PWR_BackupAccessCmd(DISABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOD, ENABLE);
    #endif
}


void sys::reset(){
    __disable_irq();
    __disable_irq();
    NVIC_SystemReset();
}

uint64_t sys::chip::get_chip_id(){
    static uint32_t chip_id[2] = {
        *(volatile uint32_t *)0x1FFFF7E8,
        *(volatile uint32_t *)0x1FFFF7EC
    };
    return ((uint64_t)chip_id[1] << 32) | chip_id[0];
}

uint32_t sys::chip::get_flash_size(){
    static uint32_t chip_flash_size = operator"" _KB(*(volatile uint16_t *)0x1FFFF7E0);
    return chip_flash_size;
}

uint32_t sys::chip::get_chip_id_crc(){

    using ymd::hal::crc;
    
    static const uint32_t chip_id_crc = [&](){
        crc.init();
        crc.clear();
        uint64_t chip_id = get_chip_id();
        return crc.update({(uint32_t)chip_id, (uint32_t)(chip_id >> 32)});
    }();
    
    return chip_id_crc;
}

uint64_t sys::chip::get_mac_address(){
    uint64_t ret = 0;
    #ifdef CHIP_HAS_RF
    FLASH_GetMACAddress(reinterpret_cast<uint8_t *>(&ret));
    #endif
    return ret;
}


#ifdef N32G45X
#define M_RCC_SYSCLK(inst) inst.sysclkFreq;
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



uint32_t sys::clock::get_system_freq(){
    M_CLOCK_TYPEDEF RCC_CLK;
    M_RCC_CLK_GETTER(&RCC_CLK);
    return M_RCC_SYSCLK(RCC_CLK);
}


uint32_t sys::clock::get_apb1_freq(){
    M_CLOCK_TYPEDEF RCC_CLK;
    M_RCC_CLK_GETTER(&RCC_CLK);
    return M_RCC_PCLK1(RCC_CLK);
}


void sys::clock::set_ahb_div(const uint8_t _div){

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

void sys::clock::set_apb1_div(const uint8_t _div){
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

void sys::clock::set_apb2_div(const uint8_t _div){

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

uint32_t sys::chip::get_dev_id(){
    return DBGMCU_GetDEVID();
}

uint32_t sys::chip::get_rev_id(){
    return DBGMCU_GetREVID();
}

uint32_t sys::clock::get_apb2_freq(){
    M_CLOCK_TYPEDEF RCC_CLK;
    M_RCC_CLK_GETTER(&RCC_CLK);
    return M_RCC_PCLK2(RCC_CLK);
}

uint32_t sys::clock::get_ahb_freq(){
    M_CLOCK_TYPEDEF RCC_CLK;
    M_RCC_CLK_GETTER(&RCC_CLK);
    return M_RCC_HCLK(RCC_CLK);
}

void sys::clock::set_ahb_freq(const uint32_t freq){
    set_ahb_div(get_ahb_freq() / freq);
}

void sys::clock::set_apb1_freq(const uint32_t freq){
    set_apb1_div(get_apb1_freq() / freq);
}

void sys::clock::set_apb2_freq(const uint32_t freq){
    set_apb2_div(get_apb2_freq() / freq);
}


bool sys::exception::is_interrupt_pending(){
    #ifdef ARCH_QKV4
    return QingKeV4::isInterruptPending();
    #else
    return false;
    #endif
}

bool sys::exception::is_intrrupt_acting(){
    #ifdef ARCH_QKV4
    return QingKeV4::isIntrruptActing();
    #else
    return false;
    #endif
}

uint8_t sys::exception::get_interrupt_depth(){
    #ifdef ARCH_QKV4
    return QingKeV4::getInterruptDepth();
    #else
    return 0;
    #endif
}

void sys::exception::disable_interrupt(){
    __disable_irq();
}

void sys::exception::enable_interrupt(){
    __enable_irq();
}

// 定义宏：读取寄存器的值
#define READ_REGISTER(reg) ({ \
    uint32_t ret; \
    __asm__ volatile ( \
        "mv %0, " #reg "\n" \
        : "=r" (ret) \
    ); \
    ret; \
})

// 定义宏：读取CSR寄存器的值
#define READ_CSR(csr) ({ \
    uint32_t ret; \
    __asm__ volatile ( \
        "csrr %0, " #csr "\n" \
        : "=r" (ret) \
    ); \
    ret; \
})

void sys::exit() {
    // 使用宏读取寄存器的值
    const uint32_t cpu_x1 = READ_REGISTER(x1);
    const uint32_t cpu_x3 = READ_REGISTER(x3);
    const uint32_t cpu_mstatus = READ_CSR(mstatus);

    // 打印寄存器值
    DEBUG_PRINTLN(
        "system exited, here is map:",
        "mstatus: ", cpu_mstatus,
        "x1: ", cpu_x1,
        "x3: ", cpu_x3
    );

    // 退出程序
    _exit(0);
}
void sys::halt(){
    HALT;
}

void sys::jumpto(const uint32_t addr){
    __asm__ volatile (
        "jmp %0"
        :
        : "r" (addr)
        : "memory"
    );
    __builtin_unreachable();
}