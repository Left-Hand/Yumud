#ifndef __SPI_HPP__

#define __SPI_HPP__

#include "src/gpio/gpio.hpp"
#include "src/gpio/port_virtual.hpp"
#include "src/bus/serbus.hpp"



class Spi:public SerBus{
protected:
    static constexpr uint8_t spi_max_cs_pins = 4;
    PortVirtual <spi_max_cs_pins> cs_pins = PortVirtual<spi_max_cs_pins>();
public:
    Error begin_use(const uint8_t & index = 0) override {
        lock(index);
        cs_pins[wholock()].clr();
        return ErrorType::OK;

    }

    void end_use() override {
        cs_pins[wholock()].set();
        unlock();
    }

    bool is_idle() override {
        return wholock() < 0;
    }

    bool owned_by(const uint8_t & index = 0) override{
        return (wholock() == index);
    }

    virtual void lock(const uint8_t & index) = 0;
    virtual void unlock() = 0;
    virtual int8_t wholock() = 0;

    void bindCsPin(GpioConcept & gpio, const uint8_t index){
        cs_pins.bindPin(gpio, index);
    }
};


class SpiHw:public Spi{
protected:
    SPI_TypeDef * instance;
    bool hw_cs_enabled = false;

    Gpio & getMosiPin();
    Gpio & getMisoPin();
    Gpio & getSclkPin();
    Gpio & getCsPin();

    void enableRcc(const bool en = true);
    uint16_t calculatePrescaler(const uint32_t baudRate);
    void initGpios();

public:
    SpiHw(SPI_TypeDef * _instance):instance(_instance){;}
    SpiHw(SPI_TypeDef * _instance, Gpio & _cs_pin):instance(_instance){bindCsPin(_cs_pin, 0);}
    SpiHw(SPI_TypeDef * _instance, GpioVirtual & _cs_pin):instance(_instance){bindCsPin(_cs_pin, 0);}
    void init(const uint32_t & baudRate, const Mode & _mode = Mode::TxRx);
    void enableHwCs(const bool en = true);

    void enableRxIt(const bool en = true);

    Error write(const uint32_t & data) override;
    Error read(uint32_t & data, bool toAck = true) override;
    Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck = true) override;

    void configDataSize(const uint8_t & data_size) override{
        uint16_t tempreg =  instance->CTLR1;
        if(data_size == 16){
            if(tempreg & SPI_DataSize_16b) return;
            tempreg |= SPI_DataSize_16b;
        }else{
            tempreg &= ~SPI_DataSize_16b;
        }
        instance->CTLR1 = tempreg;
    }

    void configBaudRate(const uint32_t & baudRate) override{
        instance->CTLR1 &= ~SPI_BaudRatePrescaler_256;
        instance->CTLR1 |= calculatePrescaler(baudRate);
    }

    void configBitOrder(const bool & msb) override {
        instance->CTLR1 &= (!SPI_FirstBit_LSB);
        instance->CTLR1 |= msb ? SPI_FirstBit_MSB : SPI_FirstBit_LSB;
    }
};

#endif