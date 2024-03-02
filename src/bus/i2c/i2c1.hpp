#ifndef __I2C1_HPP__
#define __I2C1_HPP__

#include "i2c.hpp"
#include "../../gpio/gpio.hpp"

class I2c1: public I2cHw{
private:
    volatile int8_t occupied = -1;

    Error start(const uint8_t & _address) override;

    void stop() {
        I2C_GenerateSTOP(I2C1, ENABLE);
        I2C_AcknowledgeConfig(I2C1, ENABLE);
    }


protected:

    Error begin_use(const uint8_t & index = 0) override {
        occupied = index >> 1;
        return start(index);
    }

    void reset() override;

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

    I2c1(){;}

    Error write(const uint32_t & data) override {
        I2C_SendData(I2C1, data);
        I2C_WAIT_COND(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED), timeout);
        return Bus::ErrorType::OK;
    }

    Error read(uint32_t & data, bool toAck = true) {
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

    Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
        write(data_tx);
        read(data_rx, toAck);
        return ErrorType::OK;
    }

    void init(const uint32_t & baudRate) override ;
    void enableHwTimeout(const bool en = true) override;
    void configDataSize(const uint8_t & data_size) override {;}
    void configBaudRate(const uint32_t & baudRate) override {;}
    void configBitOrder(const bool & msb) override {;}
};


#if defined(HAVE_I2C1)
extern I2c1 i2c1;
#endif

#endif
