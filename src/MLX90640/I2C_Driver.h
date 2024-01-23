#ifndef __I2C_DRIVER_H__

#define __I2C_DRIVER_H__

void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    I2C_InitTypeDef I2C_InitTSturcture = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(I2C_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = SCL_Pin;//SCL
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SDA_Pin;//SDA
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    I2C_InitTSturcture.I2C_ClockSpeed = 400000;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitTSturcture.I2C_OwnAddress1 = 0x00;//随便外设不与外设相同即可
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C_MODE, &I2C_InitTSturcture);

    I2C_Cmd(I2C_MODE, ENABLE);

    I2C_AcknowledgeConfig(I2C_MODE, ENABLE);
}


#endif