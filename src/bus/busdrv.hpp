#ifndef __SPIDRV_HPP__
#define __SPIDRV_HPP__

#include "spi/spi.hpp"
#include "i2c/i2c.hpp"
#include <type_traits>

enum class BusType{
    SpiBus,
    I2cBus
};

class BusDrv{
protected:
    Bus & bus;
    uint8_t index = 0;
    uint8_t data_size = 0;
    BusType bus_type = BusType::SpiBus;
public:
    BusDrv(I2c & _bus, const uint8_t & _index = 0):bus(_bus), index(_index){bus_type = BusType::I2cBus;}
    BusDrv(Spi & _bus, const uint8_t & _index = 0):bus(_bus), index(_index){bus_type = BusType::SpiBus;}
    
    volatile void write(const uint8_t & data){
        if(!bus.begin(index)){
            bus.write((const uint32_t &)data);
            bus.end();
        }
    }

    void write(const uint16_t & data){
        if(!bus.begin(index)){
            bus.write(data);
            bus.end();
        }
    }

    void write(const uint8_t & data0, uint8_t * data_ptr, const size_t & len){
        if(!bus.begin(index)){
            bus.write(data0);
            for(size_t i = 0; i < len; i++) bus.write(data_ptr[i]);
            bus.end();
        }
    }

    void write(const uint8_t & data0, const uint8_t & data1, const size_t & len){
        if(!bus.begin(index)){
            bus.write(data0);
            for(size_t i = 0; i < len; i++) bus.write(data1);
            bus.end();
        }
    }

    void write(uint8_t * data_ptr, const size_t & len){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++) bus.write(data_ptr[i]);
            bus.end();
        }
    }

    void write(const uint8_t & data, const size_t & len){
        if(!bus.begin(index)){
            for(size_t _ = 0; _ < len; _++) bus.write(data);
            bus.end();
        }
    }

    void write(uint16_t * data_ptr, const size_t & len){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            for(size_t i = 0; i < len; i++) bus.write(data_ptr[i]);
            bus.configDataSize(8);
            bus.end();
        }
    }

    void write(const uint16_t & data, const size_t & len){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            for(size_t _ = 0; _ < len; _++) bus.write(data);
            bus.configDataSize(8);
            bus.end();
        }
    }

    bool isBusType(const BusType & _bus_type) {
        return (_bus_type == bus_type);
    }

    bool isI2cBus(){
        return (bus_type == BusType::I2cBus);
    }

    bool isSpiBus(){
        return (bus_type == BusType::SpiBus);
    }
};

#endif