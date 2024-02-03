#ifndef __SPIDRV_HPP__
#define __SPIDRV_HPP__

#include "bus.hpp"
#include <type_traits>

class BusDrv{
protected:
    Bus & bus;
    uint8_t index = 0;
    uint8_t data_size = 0;

public:
    BusDrv(Bus & _bus, const uint8_t & _index = 0):bus(_bus), index(_index){;}
    
    void write(const uint8_t & data){
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

    template<typename T>
    bool isBusType() {
        return std::is_same<T, Bus>::value;
    }
};

#endif