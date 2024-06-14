#pragma once

#include "i2c.hpp"
#include "hal/gpio/gpio.hpp"

class I2cHw: public I2c{
private:
    void enableRcc(bool enable = true);
    Error lead(const uint8_t _address) override{
        bool is_read = (_address & 0x01);
        // while(I2C_GetFlagStatus(instance, I2C_FLAG_BUSY));
        I2C_GenerateSTART(instance, ENABLE);
        while(!I2C_CheckEvent(instance, I2C_EVENT_MASTER_MODE_SELECT) );
        I2C_Send7bitAddress(instance, _address & 0xFE, is_read ? I2C_Direction_Receiver : I2C_Direction_Transmitter);
        while(!I2C_CheckEvent(instance, is_read ? I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED :  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
        return ErrorType::OK;
    }

    void trail() override{
        I2C_GenerateSTOP(instance, ENABLE);
        I2C_AcknowledgeConfig(instance, ENABLE);
    }

protected:
    I2C_TypeDef * instance;

    Gpio & getScl();
    Gpio & getSda();
public:

    I2cHw(I2C_TypeDef * _instance):instance(_instance){;}

    Error write(const uint32_t data) override final{
        I2C_SendData(instance, data);
        while(!I2C_CheckEvent(instance, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
        return Bus::ErrorType::OK;
    }

    Error read(uint32_t & data, bool toAck = true) override final{
        uint8_t ret = 0;
        if(!toAck){
            I2C_AcknowledgeConfig(instance, DISABLE);
        }else{
            I2C_AcknowledgeConfig(instance, ENABLE);
        }
        // while(I2C_GetFlagStatus(instance, I2C_FLAG_RXNE) == RESET);
        while(!I2C_CheckEvent(instance, I2C_EVENT_MASTER_BYTE_RECEIVED));
        ret = I2C_ReceiveData(instance);
        data = ret;
        return ErrorType::OK;
    }

    Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
        write(data_tx);
        read(data_rx, toAck);
        return ErrorType::OK;
    }

    void init(const uint32_t baudRate) override ;
    void reset();
    bool locked();
    void unlock();
    void configBaudRate(const uint32_t baudRate) override{;}//TODO
    void enableHwTimeout(const bool en = true);
};

#ifdef HAVE_I2C1
extern I2cHw I2c1;
#endif

#ifdef HAVE_I2C2
extern I2cHw i2c2;
#endif
