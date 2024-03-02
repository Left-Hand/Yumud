#ifndef __I2C1_HPP__
#define __I2C1_HPP__

#include "i2c.hpp"
#include "../../gpio/gpio.hpp"

class I2c1: public I2c{
private:
    volatile int8_t occupied = -1;

    Error start(const uint8_t & _address) override{
        occupied = _address >> 1;
        bool is_read = (_address & 0x01);
        // while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
        I2C_GenerateSTART(I2C1, ENABLE);
        {
            uint32_t begin_time = micros();
            while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)){
                if(micros() - begin_time > 10000){
                    return ErrorType::NO_ACK;
                }
            }
        }
        I2C_Send7bitAddress(I2C1, _address & 0xFE, is_read ? I2C_Direction_Receiver : I2C_Direction_Transmitter);
        while(!I2C_CheckEvent(I2C1, is_read ? I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED :  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
        return ErrorType::OK;
    }

    void stop() {
        I2C_GenerateSTOP(I2C1, ENABLE);
        I2C_AcknowledgeConfig(I2C1, ENABLE);
        occupied = -1;
    }


protected :
    Error begin_use(const uint8_t & index = 0) override {return start(index);}
    void end_use() override {stop();}

    bool is_idle() override {
        return (occupied >= 0 ? false : true);
    }

    bool owned_by(const uint8_t & index = 0) override{
        return (occupied == (index >> 1));
    }

public:

    I2c1(){;}

    Error write(const uint32_t & data) override {
        I2C_SendData(I2C1, data);
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
        return Bus::ErrorType::OK;
    }

    Error read(uint32_t & data, bool toAck = true) {
        uint8_t ret = 0;
        if(!toAck){
            I2C_AcknowledgeConfig(I2C1, DISABLE);
        }else{
            I2C_AcknowledgeConfig(I2C1, ENABLE);
        }
        // while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
        ret = I2C_ReceiveData(I2C1);
        data = ret;
        return ErrorType::OK;
    }

    Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
        write(data_tx);
        read(data_rx, toAck);
        return ErrorType::OK;
    }

    void init(const uint32_t & baudRate) override ;
    void configDataSize(const uint8_t & data_size) override {;}
    void configBaudRate(const uint32_t & baudRate) override {;}
    void configBitOrder(const bool & msb) override {;}
};

extern I2c1 i2c1;

#endif
