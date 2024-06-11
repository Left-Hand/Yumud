// #include "i2c2.hpp"

// void I2C2_GPIO_Init(){
//     CHECK_INIT

//     GPIO_InitTypeDef GPIO_InitStructure;

//     RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
//     GPIO_InitStructure.GPIO_Pin = I2C2_SCL_Pin | I2C2_SDA_Pin;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

//     GPIO_Init(I2C2_Port, &GPIO_InitStructure);
// }

// void I2C2_Init(uint32_t baudRate){
//     CHECK_INIT

//     I2C_InitTypeDef I2C_InitStructure;
//     I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
//     I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
//     I2C_InitStructure.I2C_OwnAddress1 = 0;
//     I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
//     I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
//     I2C_InitStructure.I2C_ClockSpeed = baudRate;
//     I2C_Init(I2C2, &I2C_InitStructure);
//     I2C_Cmd(I2C2, ENABLE);
// }

// void I2c2::init(const uint32_t & baudRate){
//     I2C2_GPIO_Init();
//     I2C2_Init(baudRate);
// }


// #ifndef HAVE_I2C2
// #define HAVE_I2C2
// I2c2 i2c2;
// #endif