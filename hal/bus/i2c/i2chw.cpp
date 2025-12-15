#include "i2chw.hpp"
#include "hal/gpio/gpio.hpp"

#include "ral/i2c.hpp"
#include "core/sdk.hpp"

#if 0
#ifdef I2C1_PRESENT
    #if I2C1_REMAP == 0

        #define I2C1_SCL_PIN hal::PB<6>()
        #define I2C1_SDA_PIN hal::PB<7>()

    #elif I2C1_REMAP == 1

        #define I2C1_SCL_PIN hal::PB<8>()
        #define I2C1_SDA_PIN hal::PB<9>()

    #endif
#endif


#ifdef I2C2_PRESENT

    #if I2C2_REMAP == 0

    #define I2C2_SCL_PIN hal::PB<10>()
    #define I2C2_SDA_PIN hal::PB<11>()

    #endif
#endif


#endif


using namespace ymd::hal;
using namespace ymd;

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, I2C_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::I2C_Def)>(x))



static hal::Gpio i2c_get_scl(const void * inst, const uint8_t remap){
    switch(reinterpret_cast<uint32_t>(inst)){
        #ifdef I2C1_PRESENT
        case I2C1_BASE:
            return I2C1_SCL_PIN;
        #endif

        #ifdef I2C2_PRESENT
        case I2C2_BASE:
            return I2C2_SCL_PIN;
        #endif
    }

    __builtin_unreachable();
}

static hal::Gpio i2c_get_sda(const void * inst, const uint8_t remap){
    switch(reinterpret_cast<uint32_t>(inst)){
        #ifdef I2C1_PRESENT
        case I2C1_BASE:
            return I2C1_SDA_PIN;
        #endif

        #ifdef I2C2_PRESENT
        case I2C2_BASE:
            return I2C2_SDA_PIN;
        #endif
    }
    __builtin_unreachable();
}


I2cHw::I2cHw(void * inst):
    inst_(inst),
    scl_(i2c_get_scl(inst, 0)), 
    sda_(i2c_get_sda(inst, 0))
    {;}


void I2cHw::enable_rcc(const Enable en){
    switch(reinterpret_cast<uint32_t>(SDK_INST(inst_))){
        #ifdef I2C1_PRESENT
        case I2C1_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, en);
            if(I2C1_REMAP){
                GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
            }
            break;
        #endif

        #ifdef I2C2_PRESENT
        case I2C2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, en);
            break;
        #endif

        default:
            break;
    }
}



bool I2cHw::locked(){
    return bool(SDK_INST(inst_)->STAR2 & I2C_STAR2_BUSY) 
        & bool(!(SDK_INST(inst_)->STAR1 & I2C_STAR1_STOPF));
}

void I2cHw::init(const uint32_t baudrate){
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
    I2C_Init(SDK_INST(inst_), &I2C_InitStructure);
    I2C_Cmd(SDK_INST(inst_), ENABLE);

    enable_hw_timeout(EN);
}

void I2cHw::reset(){
    I2C_Cmd(SDK_INST(inst_),DISABLE);
    __nopn(4);
    I2C_Cmd(SDK_INST(inst_),ENABLE);
    __nopn(2);
    SDK_INST(inst_)->CTLR1 |= I2C_CTLR1_SWRST;
    __nopn(4);
    SDK_INST(inst_)->CTLR1 &= ~I2C_CTLR1_SWRST;
}

void I2cHw::enable_hw_timeout(const Enable en){
    if(en == EN) SDK_INST(inst_)->STAR1 |= I2C_STAR1_TIMEOUT;
    else SDK_INST(inst_)->STAR1 &= ~I2C_STAR1_TIMEOUT;
}

hal::HalResult I2cHw::unlock_bus(){
    if(locked()){
        I2C_Cmd(SDK_INST(inst_), DISABLE);

        scl().outpp();
        sda().outpp();

        volatile uint32_t _;

        for(uint8_t i = 0; i < 9; i++){

            scl().set_high();
            _ = 32;
            while(_ --);

            scl().set_low();

            _ = 32;
            while(_ --);
        }

        scl().afod();
        sda().afod();

        I2C_Cmd(SDK_INST(inst_), ENABLE);
        reset();

        return hal::HalResult::Ok();
    }

    return hal::HalResult::Ok();
}

void I2cHw::trail(){
    I2C_GenerateSTOP(SDK_INST(inst_), ENABLE);
    // I2C_AcknowledgeConfig(SDK_INST(inst_), ENABLE);
}


hal::HalResult I2cHw::lead(const hal::I2cSlaveAddrWithRw req){
    const auto address = req.addr_without_rw();
    const bool is_read = req.is_read();

    I2C_GenerateSTART(SDK_INST(inst_), ENABLE);

    while(I2C_CheckEvent(SDK_INST(inst_), I2C_EVENT_MASTER_MODE_SELECT) 
        == ErrorStatus::NoREADY);

    I2C_Send7bitAddress(SDK_INST(inst_), address & 0xFE, 
        is_read ? I2C_Direction_Receiver : I2C_Direction_Transmitter);

    while(I2C_CheckEvent(SDK_INST(inst_), is_read ? 
        I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED :  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) 
        == ErrorStatus::NoREADY);

    return hal::HalResult::Ok();
}

hal::HalResult I2cHw::write(const uint32_t data){
    I2C_SendData(SDK_INST(inst_), data);
    while(I2C_CheckEvent(SDK_INST(inst_), I2C_EVENT_MASTER_BYTE_TRANSMITTED) 
        == ErrorStatus::NoREADY);
    return hal::HalResult::Ok();
}

hal::HalResult I2cHw::read(uint8_t & data, const Ack ack){
    I2C_AcknowledgeConfig(SDK_INST(inst_), ack == ACK);
    while(I2C_GetFlagStatus(SDK_INST(inst_), I2C_FLAG_RXNE) == ErrorStatus::NoREADY);

    data = I2C_ReceiveData(SDK_INST(inst_));
    return hal::HalResult::Ok();
}

namespace ymd{

#ifdef I2C1_PRESENT
I2cHw i2c1{I2C1};
#endif

#ifdef I2C2_PRESENT
I2cHw i2c2{I2C2};
#endif

}