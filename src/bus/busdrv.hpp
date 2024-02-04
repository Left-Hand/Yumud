#ifndef __BUSDRV_HPP__
#define __BUSDRV_HPP__

#include "spi/spi.hpp"
#include "i2c/i2c.hpp"
#include <type_traits>
#include <initializer_list>

enum class BusType{
    SpiBus,
    I2cBus
};

class BusDrv{
protected:
    Bus & bus;
    uint8_t index = 0;
    BusType bus_type = BusType::SpiBus;

    uint8_t getIndex(){return index | isI2cBus();}
public:
    BusDrv(I2c & _bus, const uint8_t & _index = 0):bus(_bus), index(_index){bus_type = BusType::I2cBus;}
    BusDrv(Spi & _bus, const uint8_t & _index = 0):bus(_bus), index(_index){bus_type = BusType::SpiBus;}


    void write(const uint8_t & data, bool discont = true){
        if(!bus.begin(index)){
            bus.write(data);
            if(discont) bus.end();
        }
    }

    void write(const uint16_t & data, bool discont = true){
        if(!bus.begin(index)){
            bus.write(data);
            if(discont) bus.end();
        }
    }

    void write(std::initializer_list<const uint8_t> datas, bool discont = true){
        if(!bus.begin(index)){
            for(auto data_item : datas) bus.write(data_item);
            if(discont) bus.end();
        }
    }

    void write(const uint8_t & data, const size_t & len, bool discont = true){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++) bus.write(data);
            if(discont) bus.end();
        }
    }

    void write(const uint16_t & data, const size_t & len, bool discont = true){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++) bus.write(data);
            if(discont) bus.end();
        }
    }

    void write(uint8_t * data_ptr, const size_t & len, bool discont = true){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            for(size_t i = 0; i < len; i++) bus.write(data_ptr[i]);
            bus.configDataSize(8);
            if(discont) bus.end();
        }
    }
    
    void write(uint16_t * data_ptr, const size_t & len, bool discont = true){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            for(size_t i = 0; i < len; i++) bus.write(data_ptr[i]);
            bus.configDataSize(8);
            if(discont) bus.end();
        }
    }

    void read(uint8_t * data_ptr, const size_t & len){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++){
                uint32_t & temp = (uint32_t &)(data_ptr[i]);
                bus.read(temp, (i != len - 1));
            }
            bus.end();
        }
    }

    void read(uint8_t & data){
        if(!bus.begin(index)){
            uint32_t & temp = (uint32_t &)data;
            bus.read(temp);
            bus.end();
        }
    }

    void read(uint16_t & data){
        read((uint8_t *)(&data), 2);
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