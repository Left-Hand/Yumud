#include "i2chw.hpp"
#include "hal/gpio/port.hpp"

using namespace ymd::hal;
using namespace ymd;

void I2cHw::enableRcc(const bool en){
    switch((uint32_t)instance){
        #ifdef ENABLE_I2C1
        case I2C1_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, en);
            if(I2C1_REMAP){
                GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
            }
            break;
        #endif

        #ifdef ENABLE_I2C2
        case I2C2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, en);
            break;
        #endif

        default:
            break;
    }
}


hal::GpioIntf & I2cHw::getScl(const I2C_TypeDef * _instance){
    switch((uint32_t)_instance){
        #ifdef ENABLE_I2C1
        case I2C1_BASE:
            return I2C1_SCL_GPIO;
        #endif

        #ifdef ENABLE_I2C2
        case I2C2_BASE:
            return I2C2_SCL_GPIO;
        #endif

        default:
            return NullGpio;
    }
}

hal::GpioIntf & I2cHw::getSda(const I2C_TypeDef * _instance){
    switch((uint32_t)_instance){
        #ifdef ENABLE_I2C1
        case I2C1_BASE:
            return I2C1_SDA_GPIO;
        #endif

        #ifdef ENABLE_I2C2
        case I2C2_BASE:
            return I2C2_SDA_GPIO;
        #endif

        default:
            return NullGpio;
    }
}

bool I2cHw::locked(){
    return (bool)(instance->STAR2 & I2C_STAR2_BUSY) & (bool)(!(instance->STAR1 & I2C_STAR1_STOPF));
}

void I2cHw::init(const uint32_t baudRate){
    // preinit();
    enableRcc();

    scl_gpio.afod();
    sda_gpio.afod();

    I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x80;
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

void I2cHw::unlock_bus(){
    if(locked()){
        I2C_Cmd(instance, DISABLE);

        scl_gpio.outpp();
        sda_gpio.outpp();

        volatile uint32_t _;

        for(uint8_t i = 0; i < 9; i++){

            scl_gpio = true;
            _ = 32;
            while(_ --);

            scl_gpio = false;

            _ = 32;
            while(_ --);
        }

        scl_gpio.afod();
        sda_gpio.afod();

        I2C_Cmd(instance, ENABLE);
        reset();
    }
}

void I2cHw::trail(){
    I2C_GenerateSTOP(instance, ENABLE);
    // I2C_AcknowledgeConfig(instance, ENABLE);
}


BusError I2cHw::lead(const uint8_t _address){
    bool is_read = (_address & 0x01);
    // while(I2C_GetFlagStatus(instance, I2C_FLAG_BUSY));
    I2C_GenerateSTART(instance, ENABLE);
    while(I2C_CheckEvent(instance, I2C_EVENT_MASTER_MODE_SELECT) == ErrorStatus::NoREADY);
    I2C_Send7bitAddress(instance, _address & 0xFE, is_read ? I2C_Direction_Receiver : I2C_Direction_Transmitter);
    while(I2C_CheckEvent(instance, is_read ? I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED :  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ErrorStatus::NoREADY);
    return BusError::OK;
}

BusError I2cHw::write(const uint32_t data){
    I2C_SendData(instance, data);
    while(I2C_CheckEvent(instance, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ErrorStatus::NoREADY);
    return BusError::OK;
}

BusError I2cHw::read(uint32_t & data, const Ack ack){
    I2C_AcknowledgeConfig(instance, bool(ack));
    while(I2C_GetFlagStatus(instance, I2C_FLAG_RXNE) == ErrorStatus::NoREADY);
    // while(!I2C_CheckEvent(instance, I2C_EVENT_MASTER_BYTE_RECEIVED));
    data = I2C_ReceiveData(instance);
    return BusError::OK;
}

namespace ymd{

#ifdef ENABLE_I2C1
I2cHw i2c1{I2C1};
#endif

#ifdef ENABLE_I2C2
I2cHw i2c2{I2C2};
#endif

}