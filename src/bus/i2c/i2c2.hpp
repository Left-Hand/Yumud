#ifndef __I2C2_HPP__
#define __I2C2_HPP__

#include "i2c.hpp"
#include "../../gpio/gpio.hpp"

class I2c2: public I2c{
private:
    volatile int8_t occupied = -1;

    Error start(const uint8_t & _address) {
        bool is_read = (_address & 0x01);
        // while(I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY));
        I2C_GenerateSTART(I2C2, ENABLE);
        while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT) );
        I2C_Send7bitAddress(I2C2, _address & 0xFE, is_read ? I2C_Direction_Receiver : I2C_Direction_Transmitter);
        while(!I2C_CheckEvent(I2C2, is_read ? I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED :  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
        return ErrorType::OK;
    }

    void stop() {
        I2C_GenerateSTOP(I2C2, ENABLE);
        I2C_AcknowledgeConfig(I2C2, ENABLE);
    }


protected :
    Error begin_use(const uint8_t & index = 0) override {
        occupied = index >> 1;
        return start(index);
    }

    void end_use() override {
        stop();
        occupied = -1;
    }

    bool is_idle() override {
        return (occupied >= 0 ? false : true);
    }

    bool owned_by(const uint8_t & index = 0) override{
        return (occupied == (index >> 1));
    }
public:

    I2c2(){;}

    Error write(const uint32_t & data) override {
        I2C_SendData(I2C2, data);
        while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
        return Bus::ErrorType::OK;
    }

    Error read(uint32_t & data, bool toAck = true) {
        uint8_t ret = 0;
        if(!toAck){
            I2C_AcknowledgeConfig(I2C2, DISABLE);
        }else{
            I2C_AcknowledgeConfig(I2C2, ENABLE);
        }
        // while(I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE) == RESET);
        while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED));
        ret = I2C_ReceiveData(I2C2);
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

extern I2c2 i2c2;

#endif
