#include "tb.h"

static constexpr size_t n = 40;
static std::array<uint16_t, n> data;


[[maybe_unused]] static uint16_t m_crc(uint16_t data_in){
	uint16_t speed_data;
	speed_data = data_in << 5;
	data_in = data_in << 1;
	data_in = (data_in ^ (data_in >> 4) ^ (data_in >> 8)) & 0x0f;
	
	return speed_data | data_in;
}
	

[[maybe_unused]]static void transfer(uint16_t data_in){
	uint8_t i;
	for(i=0;i<16;i++)
	{
		if(data_in & 0x8000)data[i] = 171;
		else data[i] = 81;
		data_in = data_in << 1;
	}
}

void TIM1_PWMOut_Init(u16 arr, u16 psc, u16 ccp)
{
    GPIO_InitTypeDef        GPIO_InitStructure = {0};
    TIM_OCInitTypeDef       TIM_OCInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM8, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = ccp;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM8, &TIM_OCInitStructure);
    TIM_OC2Init(TIM8, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM8, ENABLE);
}

/*********************************************************************
 * @fn      TIM1_DMA_Init
 *
 * @brief   Initializes the TIM DMAy Channelx configuration.
 *
 * @param   DMA_CHx -
 *            x can be 1 to 7.
 *          ppadr - Peripheral base address.
 *          memadr - Memory base address.
 *          bufsize - DMA channel buffer size.
 *
 * @return  none
 */
void TIM1_DMA_Init(DMA_Channel_TypeDef *DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize)
{
    DMA_InitTypeDef DMA_InitStructure = {0};

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

    DMA_DeInit(DMA_CHx);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr;
    DMA_InitStructure.DMA_MemoryBaseAddr = memadr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = bufsize;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA_CHx, &DMA_InitStructure);

    DMA_Cmd(DMA_CHx, ENABLE);
}

void dshot_tb(OutputStream & logger, TimerOC & oc1, TimerOC & oc2){


    // dma2Ch3.init(DmaChannel::Mode::toPeriphCircular, DmaChannel::Priority::ultra);
    // dma2Ch3.begin((void *)&oc.cnt(), (const void *)data.begin(), data.size());
    // dma2Ch3.configDataBytes(2);
    // oc.enableDma();

    while(true){
        // transfer(m_crc(0));
        // oc.cvr() = int(220 * (sin(t) * 0.5 + 0.5));
        oc1 = sin(t) * 0.5 + 0.5;
        oc2 = sin(t) * 0.5 + 0.5;
        delay(1);
        logger.println(oc1.cvr(), oc1.arr(), oc2.cvr(), oc2.arr());
        // // Delay_Ms(15000);
        // delay(100);
        // transfer(m_crc(300));
        // Delay_Ms(1000);
        // delay(100);
        // transfer(m_crc(1500));
        // delay(100);
    }
}

[[maybe_unused]]static void dshot_tb2(OutputStream & logger, TimerOC & oc){
    while(true){
        real_t val = 0.5 * sin(t) + 0.5;
        oc = val;
        logger.println(val);
    }
}

void dshot_main(){
        // USART_Printf_Init(115200);
    // Delay_Init();
    // SystemCoreClockUpdate();
    // UART1_TX_Gpio.
    auto & logger = uart1;
    logger.init(921600);
    const uint32_t freq = 200;
    timer1.init(freq);
    timer8.init(freq);
    auto & oc = timer1.oc(1);
    auto & oc2 = timer8.oc(1);
    oc.init();
    oc2.init();
    dshot_tb(logger,oc, oc2);
    // TIM1_PWMOut_Init(234-1, 0, 0);
    // TIM1_DMA_Init(DMA2_Channel3, (u32)&(TIM8->CH1CVR), (u32)data.begin(), 40);
    // // TIM1_DMA_Init(DMA2_Channel5, (u32)  0x40013438, (u32)data, 40);
    // TIM_DMACmd(TIM8, TIM_DMA_CC1, ENABLE);
    // // TIM_DMACmd(TIM8, TIM_DMA_CC2, ENABLE);
    // TIM_Cmd(TIM8, ENABLE);
    // TIM_CtrlPWMOutputs(TIM8, ENABLE);
    // TIM1_PWMOut_Init(234-1, 0, 0);
    // TIM1_DMA_Init(DMA2_Channel3, (u32)&(TIM8->CH1CVR), (u32)data.begin(), 40);
    // TIM1_DMA_Init(DMA2_Channel5, (u32)&(TIM8->CH2CVR), (u32)data.begin(), 40);
    // TIM_DMACmd(TIM8, TIM_DMA_CC1, ENABLE);
    // TIM_DMACmd(TIM8, TIM_DMA_CC2, ENABLE);
    // TIM_Cmd(TIM8, ENABLE);
    // TIM_CtrlPWMOutputs(TIM8, ENABLE);
    // 144000000/233


    // uart1.init(576000, CommMethod::Blocking);
    // auto & logger = uart1;
    // logger.setEps(4);
    // logger.setRadix(10);
    // logger.setSpace(",");

    // timer1.init(10000);

    // auto & oc = timer1.oc(1);
    // oc.init();

    // dshot_tb(logger, oc);
}