#include "misc.h"

void GPIO_PortC_Init( void ){
    CHECK_INIT

    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE );

    PWR_BackupAccessCmd( ENABLE );
    RCC_LSEConfig( RCC_LSE_OFF );
    BKP_TamperPinCmd(DISABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13| GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );
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

void SysInfo_ShowUp(){
    RCC_ClocksTypeDef RCC_CLK;
	RCC_GetClocksFreq(&RCC_CLK);//Get chip frequencies

    uart1.setSpace(" ");

    uart1.println("\r\n\r\n------------------------");
	uart1.println("System Clock Source : ", (int)RCC_GetSYSCLKSource());
	uart1.println("APB1/PCLK1 : ", (int)RCC_CLK.PCLK1_Frequency, "Hz");
	uart1.println("APB2/PCLK2 : ", (int)RCC_CLK.PCLK2_Frequency, "Hz");
	uart1.println("SYSCLK     : ", (int)RCC_CLK.SYSCLK_Frequency, "Hz");
	uart1.println("HCLK       : ", (int)RCC_CLK.HCLK_Frequency, "Hz");

    uint16_t flash_size = *(volatile uint16_t *)0x1FFFF7E0;
    uint32_t chip_id[2];
    chip_id[0] = *(volatile uint32_t *)0x1FFFF7E8;
    chip_id[1] = *(volatile uint32_t *)0x1FFFF7EC;

	uart1.println("FlashSize       : ", (int)flash_size, "KB");
    uart1 << SpecToken::Hex;
    uart1.println("ChipID          : ", (uint64_t)chip_id[0], chip_id[1]);

    RCC_AHBPeriphClockCmd(RCC_CRCEN, ENABLE);
    CRC_ResetDR();
    uint32_t crc_code = CRC_CalcBlockCRC(chip_id, 3);
    uart1.println("CRC code:", crc_code);
    uart1 << SpecToken::Dec;

    Bkp & bkp = Bkp::getInstance();
	auto boot_count = bkp.readData(1);
    bkp.writeData(1, boot_count + 1);
    uart1.println("System boot times: ", boot_count);
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