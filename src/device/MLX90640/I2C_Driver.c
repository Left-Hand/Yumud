#include "I2C_Driver.h"

void I2C2_Init(uint8_t address, uint16_t bound){
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    I2C_InitTypeDef  I2C_InitTSturcture = {0};
 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
 
    I2C_InitTSturcture.I2C_ClockSpeed = bound;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitTSturcture.I2C_OwnAddress1 = address;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C2, &I2C_InitTSturcture);
 
    I2C_Cmd(I2C2, ENABLE);

    I2C_AcknowledgeConfig(I2C2, ENABLE);
}

uint8_t I2C_Mem_Read(I2C_TypeDef * I2Cx, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout){
    uint32_t tickStart = 0;
    tickStart = millis(); // 获取当前的tick值，例如使用SysTick或其他定时器

    // 1. 发送起始信号
    I2C_GenerateSTART(I2Cx, ENABLE);

    // 2. 确认是否已经进入主模式
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) {
        if((millis() - tickStart) > Timeout) { // 替换millis()为获取当前tick值的函数
            return 1;
        }
    }

    // 3. 发送设备地址 + 写信号，以设置内存地址
    I2C_Send7bitAddress(I2Cx, DevAddress, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if((millis() - tickStart) > Timeout) {
            return 1;
        }
    }

    // 4. 发送内存地址
    if(MemAddSize == I2C_MEMADD_SIZE_8BIT) {
        I2C_SendData(I2Cx, (uint8_t)MemAddress);
    } else {
        I2C_SendData(I2Cx, (uint8_t)(MemAddress >> 8));
        // 等待数据发送完成
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            if((millis() - tickStart) > Timeout) {
                return 1;
            }
        }
        I2C_SendData(I2Cx, (uint8_t)(MemAddress & 0xFF));
    }

    // 等待数据发送完成
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if((millis() - tickStart) > Timeout) {
            return 1;
        }
    }

    // 5. 发送重复起始信号
    I2C_GenerateSTART(I2Cx, ENABLE);
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) {
        if((millis() - tickStart) > Timeout) {
            return 1;
        }
    }

    // 6. 发送设备地址 + 读信号
    I2C_Send7bitAddress(I2Cx, DevAddress, I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
        if((millis() - tickStart) > Timeout) {
            return 1;
        }
    }

    // 7. 读取数据
    while(Size) {
        if(Size == 1) {
            // 如果是最后一个字节
            I2C_AcknowledgeConfig(I2Cx, DISABLE);
            I2C_GenerateSTOP(I2Cx, ENABLE);
        }

        // 等待数据就绪
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
            if((millis() - tickStart) > Timeout) {
                return 1;
            }
        }

        // 读取数据
        *pData++ = I2C_ReceiveData(I2Cx);
        Size--;

        if(Size == 0) {
            // 确保在读取完数据后发送停止信号
            I2C_GenerateSTOP(I2Cx, ENABLE);
        }
    }

    // 8. 重新使能应答
    I2C_AcknowledgeConfig(I2Cx, ENABLE);

    return 0;
}

uint8_t I2C_Mem_Write(I2C_TypeDef* I2Cx, uint8_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout) {
    uint32_t tickStart = 0;
    tickStart = millis(); // 获取当前的tick值，例如使用SysTick或其他定时器

    // 1. 发送起始信号
    I2C_GenerateSTART(I2Cx, ENABLE);

    // 2. 确认是否已经进入主模式
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) {
        if((millis() - tickStart) > Timeout) { // 替换millis()为获取当前tick值的函数
            return 1;
        }
    }

    // 3. 发送设备地址 + 写信号
    I2C_Send7bitAddress(I2Cx, DevAddress, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if((millis() - tickStart) > Timeout) {
            return 1;
        }
    }

    // 4. 发送内存地址
    if(MemAddSize == I2C_MEMADD_SIZE_8BIT) {
        I2C_SendData(I2Cx, (uint8_t)MemAddress);
    } else {
        I2C_SendData(I2Cx, (uint8_t)(MemAddress >> 8));
        // 等待数据发送完成
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            if((millis() - tickStart) > Timeout) {
                return 1;
            }
        }
        I2C_SendData(I2Cx, (uint8_t)(MemAddress & 0xFF));
    }

    // 等待数据发送完成
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if((millis() - tickStart) > Timeout) {
            return 1;
        }
    }

    // 5. 发送数据
    while(Size--) {
        I2C_SendData(I2Cx, *pData++);
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            if((millis() - tickStart) > Timeout) {
                return 1;
            }
        }
    }

    // 6. 发送停止信号
    I2C_GenerateSTOP(I2Cx, ENABLE);

    return 0;
}
