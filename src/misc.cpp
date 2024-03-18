#include "misc.h"

void GPIO_PortC_Init( void ){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );
    #ifdef HAVE_GPIOD
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE );
    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
    #endif
    PWR_BackupAccessCmd( ENABLE );
    RCC_LSEConfig( RCC_LSE_OFF );
    BKP_TamperPinCmd(DISABLE);
    PWR_BackupAccessCmd(DISABLE);
}

void GPIO_SW_I2C_Init(void){
    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );

    GPIO_InitStructure.GPIO_Pin = I2C_SW_SCL | I2C_SW_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_WriteBit(GPIOB, GPIO_InitStructure.GPIO_Pin, Bit_SET);
    GPIO_Init( GPIOB, &GPIO_InitStructure );
}

void GPIO_SW_I2S_Init(void){
    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );

    GPIO_InitStructure.GPIO_Pin = I2S_SW_SDA | I2S_SW_SCK | I2S_SW_WS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_WriteBit(GPIOB, GPIO_InitStructure.GPIO_Pin, Bit_SET);
    GPIO_Init( GPIOB, &GPIO_InitStructure );
}

void GLobal_Reset(void){
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = RES_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(RES_PORT, &GPIO_InitStructure );

    delayMicroseconds(50);
    GPIO_WriteBit(RES_PORT, RES_PIN, Bit_RESET);
    delayMicroseconds(50);
    GPIO_WriteBit(RES_PORT, RES_PIN, Bit_SET);
}



real_t CalculateFps(){
    static real_t begin_t;
    real_t dt = t - begin_t;
    begin_t = t;
    return dt ? real_t(1) / dt : real_t(0);
}

uint64_t getChipId(){
    uint32_t chip_id[2];
    chip_id[0] = *(volatile uint32_t *)0x1FFFF7E8;
    chip_id[1] = *(volatile uint32_t *)0x1FFFF7EC;
    return ((uint64_t)chip_id[1] << 32) | chip_id[0];
}
void SysInfo_ShowUp(Printer & uart){
    RCC_ClocksTypeDef RCC_CLK;
	RCC_GetClocksFreq(&RCC_CLK);//Get chip frequencies

    uart.setSpace(" ");

    uart.println("\r\n\r\n------------------------");
	uart.println("System Clock Source : ", (int)RCC_GetSYSCLKSource());
	uart.println("APB1/PCLK1 : ", (int)RCC_CLK.PCLK1_Frequency, "Hz");
	uart.println("APB2/PCLK2 : ", (int)RCC_CLK.PCLK2_Frequency, "Hz");
	uart.println("SYSCLK     : ", (int)RCC_CLK.SYSCLK_Frequency, "Hz");
	uart.println("HCLK       : ", (int)RCC_CLK.HCLK_Frequency, "Hz");

    uint16_t flash_size = *(volatile uint16_t *)0x1FFFF7E0;
    uint32_t chip_id[2];
    chip_id[0] = *(volatile uint32_t *)0x1FFFF7E8;
    chip_id[1] = *(volatile uint32_t *)0x1FFFF7EC;

	uart.println("FlashSize       : ", (int)flash_size, "KB");
    uart << SpecToken::Hex;
    uart.println("ChipID          : ", (uint64_t)chip_id[0], chip_id[1]);

    RCC_AHBPeriphClockCmd(RCC_CRCEN, ENABLE);
    CRC_ResetDR();
    uint32_t crc_code = CRC_CalcBlockCRC(chip_id, 3);
    uart.println("CRC code:", crc_code);
    uart << SpecToken::Dec;

    // Bkp & bkp = Bkp::getInstance();
    bkp.init();
	auto boot_count = bkp.readData(1);
    bkp.writeData(1, boot_count + 1);
    uart.println("System boot times: ", boot_count);
}

void Systick_Init(void){
    static uint8_t initd = 0;
    if(initd) return;
    initd = 1;

    tick_per_ms = SystemCoreClock / 1000;
    tick_per_us = tick_per_ms / 1000;
    SysTick->SR  = 0;
    SysTick->CTLR= 0;
    SysTick->CNT = 0;
    SysTick->CMP = tick_per_ms - 1;
    SysTick->CTLR= 0xF;

    NVIC_SetPriority(SysTicK_IRQn,0xFF);
    NVIC_EnableIRQ(SysTicK_IRQn);

}

void TIM2_GPIO_Init(){
    //PA0 CH1
    //PA1 CH2
    //PA2 CH3
    //PA3 CH4
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1);
}

void TIM3_GPIO_Init(){
    //PA6 CH1
    //PA7 CH2
    //PB0 CH3
    //PB1 CH4
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);
}

void TIM4_GPIO_Init(){
    //PB6 CH1
    //PB7 CH2
    //PB8 CH3
    //PB9 CH4
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinRemapConfig(GPIO_Remap_TIM4, DISABLE);
}

void TIM_RCC_On(TIM_TypeDef * TimBase){
    if (TimBase == TIM1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    } else if (TimBase == TIM2) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    } else if (TimBase == TIM3) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    } else if (TimBase == TIM4){
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    }
}
void TIM_Encoder_Init(TIM_TypeDef * TimBase){
    TIM_RCC_On(TimBase);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TimBase,&TIM_TimeBaseStructure);

	TIM_ICInitTypeDef TIM_ICInitStruct;
    TIM_ICStructInit(&TIM_ICInitStruct);

	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStruct.TIM_ICFilter = 0xF;


	TIM_ICInit(TimBase,&TIM_ICInitStruct);

	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;

	TIM_ICInit(TimBase,&TIM_ICInitStruct);

	TIM_EncoderInterfaceConfig(TimBase,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising,TIM_ICPolarity_Rising);

    TIM_Cmd(TimBase, ENABLE);
}

void TIM_PWM_Init(TIM_TypeDef * TimBase, const uint16_t arr){
    TIM_RCC_On(TimBase);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;
    TIM_TimeBaseInit(TimBase,&TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = arr >> 1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TimBase,&TIM_OCInitStructure);
    TIM_OC2Init(TimBase,&TIM_OCInitStructure);
    TIM_OC3Init(TimBase,&TIM_OCInitStructure);
    TIM_OC4Init(TimBase,&TIM_OCInitStructure);

    TIM_ARRPreloadConfig(TimBase, DISABLE);

    TIM_Cmd(TimBase, ENABLE);
}

void ADC1_GPIO_Init(){
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_3);
    // GPIO_SetBits(GPIOA, GPIO_Pin_2);
}

void LED_GPIO_Init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_6);
}


void ADC1_Init(){
    CHECK_INIT

    ADC_InitTypeDef  ADC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_InitStructure.ADC_Pga = ADC_Pga_1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 1, ADC_SampleTime_28Cycles5);

    ADC_InjectedSequencerLengthConfig(ADC1, 2);
    ADC_InjectedChannelConfig(ADC1,ADC_Channel_2,1,ADC_SampleTime_28Cycles5);
    ADC_InjectedChannelConfig(ADC1,ADC_Channel_3,2,ADC_SampleTime_28Cycles5);

    ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_None);
    ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);

    ADC_DMACmd(ADC1, DISABLE);

    ADC_Cmd(ADC1, ENABLE);

    ADC_BufferCmd(ADC1, DISABLE);
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    ADC_BufferCmd(ADC1, ENABLE);
}

