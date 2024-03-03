#include "i2c1.hpp"

void I2C1_GPIO_Init(){
    CHECK_INIT

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Pin = I2C1_SCL_Pin | I2C1_SDA_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(I2C1_Port, &GPIO_InitStructure);
    GPIO_PinRemapConfig(I2C1_REMAP, I2C1_REMAP_ENABLE);
}

void I2C1_Init(uint32_t baudRate){
    CHECK_INIT

    I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = baudRate;
    I2C_Init(I2C1, &I2C_InitStructure);
    I2C_Cmd(I2C1, ENABLE);
}

void I2C1_HW_Timeout(FunctionalState en){
    // if(en) I2C1->STAR1 |= I2C_STAR1_TIMEOUT;
    // else I2C1->STAR1 &= ~I2C_STAR1_TIMEOUT;
}

bool I2C1_Bus_Locked(){
    return (bool)(I2C1->STAR2 & I2C_STAR2_BUSY) & (bool)(!(I2C1->STAR1 & I2C_STAR1_STOPF));
}

void I2C1_Force_Unlock(){
    I2C_Cmd(I2C1, DISABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = I2C1_SCL_Pin | I2C1_SDA_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(I2C1_Port, &GPIO_InitStructure);

    volatile uint32_t _;
    for(uint8_t i = 0; i < 9; i++){
        GPIO_SetBits(I2C1_Port, I2C1_SCL_Pin);

        _ = 32;
        while(_ --);

        GPIO_ResetBits(I2C1_Port, I2C1_SCL_Pin);

        _ = 32;
        while(_ --);
    }

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(I2C1_Port, &GPIO_InitStructure);

    I2C_Cmd(I2C1, ENABLE);
}


void I2C1_Reset(){
    I2C_Cmd(I2C1,DISABLE);
    __nopn(4);
    I2C_Cmd(I2C1,ENABLE);
    __nopn(2);
    I2C1->CTLR1 |= I2C_CTLR1_SWRST;
    __nopn(4);
    I2C1->CTLR1 &= ~I2C_CTLR1_SWRST;
}

I2c1::Error I2c1::start(const uint8_t & _address) {
    bool is_read = (_address & 0x01);
    // while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C1, ENABLE);
    I2C_WAIT_COND(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT), timeout);

    I2C_Send7bitAddress(I2C1, _address & 0xFE, is_read ? I2C_Direction_Receiver : I2C_Direction_Transmitter);
    I2C_WAIT_COND(I2C_CheckEvent(I2C1,
            is_read ? I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED :  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED),
            timeout);

    return ErrorType::OK;
}

I2c1::Error I2c1::write(const uint32_t & data){
    I2C_SendData(I2C1, data);
    I2C_WAIT_COND(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED), timeout);
    return Bus::ErrorType::OK;
}


I2c1::Error I2c1::read(uint32_t & data, bool toAck) {
    uint8_t ret = 0;
    if(!toAck){
        I2C_AcknowledgeConfig(I2C1, DISABLE);
    }else{
        I2C_AcknowledgeConfig(I2C1, ENABLE);
    }
    I2C_WAIT_COND(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED), timeout);
    ret = I2C_ReceiveData(I2C1);
    data = ret;
    return ErrorType::OK;
}


void I2c1::stop() {
    I2C_GenerateSTOP(I2C1, ENABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
}

void I2c1::init(const uint32_t & baudRate){
    I2C1_GPIO_Init();
    I2C1_Init(baudRate);
    I2C1_HW_Timeout(true);
}

void I2c1::reset(){
    I2C1_Reset();
}

void I2c1::enableHwTimeout(const bool en){
    I2C1_HW_Timeout(en);
}

void I2c1::lock_avoid(){
    if(I2C1_Bus_Locked()){
        I2C1_Force_Unlock();
        I2C1_Reset();
    }
}

#if defined(HAVE_I2C1) && !defined(HAD_I2C1)
#define HAD_I2C1
I2c1 i2c1;
#endif