#include "i2chw.hpp"
#include "hal/gpio/gpio.hpp"

using namespace ymd::hal;
using namespace ymd;

void I2cHw::enable_rcc(const Enable en){
    switch(reinterpret_cast<uint32_t>(inst_)){
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


hal::Gpio & I2cHw::get_scl(const I2C_TypeDef * inst){
    switch(reinterpret_cast<uint32_t>(inst)){
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

hal::Gpio & I2cHw::get_sda(const I2C_TypeDef * inst){
    switch(reinterpret_cast<uint32_t>(inst)){
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
    return bool(inst_->STAR2 & I2C_STAR2_BUSY) 
        & bool(!(inst_->STAR1 & I2C_STAR1_STOPF));
}

void I2cHw::init(const uint32_t baudrate){
    // preinit();
    enable_rcc(EN);

    scl().afod();
    sda().afod();

    I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x80;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = baudrate;
    I2C_Init(inst_, &I2C_InitStructure);
    I2C_Cmd(inst_, ENABLE);

    enable_hw_timeout(EN);
}

void I2cHw::reset(){
    I2C_Cmd(inst_,DISABLE);
    __nopn(4);
    I2C_Cmd(inst_,ENABLE);
    __nopn(2);
    inst_->CTLR1 |= I2C_CTLR1_SWRST;
    __nopn(4);
    inst_->CTLR1 &= ~I2C_CTLR1_SWRST;
}

void I2cHw::enable_hw_timeout(const Enable en){
    if(en == EN) inst_->STAR1 |= I2C_STAR1_TIMEOUT;
    else inst_->STAR1 &= ~I2C_STAR1_TIMEOUT;
}

hal::HalResult I2cHw::unlock_bus(){
    if(locked()){
        I2C_Cmd(inst_, DISABLE);

        scl().outpp();
        sda().outpp();

        volatile uint32_t _;

        for(uint8_t i = 0; i < 9; i++){

            scl().set();
            _ = 32;
            while(_ --);

            scl().clr();

            _ = 32;
            while(_ --);
        }

        scl().afod();
        sda().afod();

        I2C_Cmd(inst_, ENABLE);
        reset();

        return hal::HalResult::Ok();
    }

    return hal::HalResult::Ok();
}

void I2cHw::trail(){
    I2C_GenerateSTOP(inst_, ENABLE);
    // I2C_AcknowledgeConfig(inst_, ENABLE);
}


hal::HalResult I2cHw::lead(const LockRequest req){
    const auto address = req.id();
    const bool is_read = req.custom();
    // while(I2C_GetFlagStatus(inst_, I2C_FLAG_BUSY));
    I2C_GenerateSTART(inst_, ENABLE);

    while(I2C_CheckEvent(inst_, I2C_EVENT_MASTER_MODE_SELECT) 
        == ErrorStatus::NoREADY);

    I2C_Send7bitAddress(inst_, address & 0xFE, 
        is_read ? I2C_Direction_Receiver : I2C_Direction_Transmitter);

    while(I2C_CheckEvent(inst_, is_read ? 
        I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED :  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) 
        == ErrorStatus::NoREADY);

    return hal::HalResult::Ok();
}

hal::HalResult I2cHw::write(const uint32_t data){
    I2C_SendData(inst_, data);
    while(I2C_CheckEvent(inst_, I2C_EVENT_MASTER_BYTE_TRANSMITTED) 
        == ErrorStatus::NoREADY);
    return hal::HalResult::Ok();
}

hal::HalResult I2cHw::read(uint32_t & data, const Ack ack){
    I2C_AcknowledgeConfig(inst_, ack == ACK);
    while(I2C_GetFlagStatus(inst_, I2C_FLAG_RXNE) == ErrorStatus::NoREADY);

    data = I2C_ReceiveData(inst_);
    return hal::HalResult::Ok();
}

namespace ymd{

#ifdef ENABLE_I2C1
I2cHw i2c1{I2C1};
#endif

#ifdef ENABLE_I2C2
I2cHw i2c2{I2C2};
#endif

}