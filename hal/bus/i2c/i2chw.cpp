#include "i2chw.hpp"
#include "hal/gpio/port.hpp"

void I2cHw::enableRcc(const bool en){
    switch((uint32_t)instance){
        #ifdef HAVE_I2C1
        case I2C1_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, en);
            GPIO_PinRemapConfig(I2C1_REMAP, I2C1_REMAP_ENABLE);
            break;
        #endif

        #ifdef HAVE_I2C2
        case I2C2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, en);
            break;
        #endif

        default:
            break;
    }
}


Gpio & I2cHw::getScl(){
    switch((uint32_t)instance){
        #ifdef HAVE_I2C1
        case I2C1_BASE:
            return I2C1_Port[Pin(I2C1_SCL_Pin)];
        #endif

        #ifdef HAVE_I2C2
        case I2C2_BASE:
            return I2C2_Port[Pin(I2C1_SCL_Pin)];
        #endif

        default:
            return GpioNull;
    }
}

Gpio & I2cHw::getSda(){
    switch((uint32_t)instance){
        #ifdef HAVE_I2C1
        case I2C1_BASE:
            return I2C1_Port[Pin(I2C1_SDA_Pin)];
        #endif

        #ifdef HAVE_I2C2
        case I2C2_BASE:
            return I2C2_Port[Pin(I2C1_SDA_Pin)];
        #endif

        default:
            return GpioNull;
    }
}

bool I2cHw::locked(){
    return (bool)(instance->STAR2 & I2C_STAR2_BUSY) & (bool)(!(instance->STAR1 & I2C_STAR1_STOPF));
}

void I2cHw::init(const uint32_t baudRate){
    getScl().OutAfOD();
    getSda().OutAfOD();

    I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = baudRate;
    I2C_Init(instance, &I2C_InitStructure);
    I2C_Cmd(instance, ENABLE);

    enableHwTimeout(true);
}

void I2cHw::reset(){
    I2C_Cmd(instance,DISABLE);
    __nopn(4);
    I2C_Cmd(instance,ENABLE);
    __nopn(2);
    instance->CTLR1 |= I2C_CTLR1_SWRST;
    __nopn(4);
    instance->CTLR1 &= ~I2C_CTLR1_SWRST;
}

void I2cHw::enableHwTimeout(const bool en){
    if(en) instance->STAR1 |= I2C_STAR1_TIMEOUT;
    else instance->STAR1 &= ~I2C_STAR1_TIMEOUT;
}

void I2cHw::unlock(){
    if(locked()){
        I2C_Cmd(instance, DISABLE);

        auto & scl = getScl();
        auto & sda = getSda();

        scl.OutPP();
        sda.OutPP();

        volatile uint32_t _;

        for(uint8_t i = 0; i < 9; i++){

            scl = true;
            _ = 32;
            while(_ --);

            scl = false;

            _ = 32;
            while(_ --);
        }

        scl.OutAfOD();
        sda.OutAfOD();

        I2C_Cmd(instance, ENABLE);
        reset();
    }
}

#if defined(HAVE_instance)
I2cHw i2c1{I2C1};
#endif

#if defined(HAVE_I2C2)
I2cHw i2c2{I2C2};
#endif