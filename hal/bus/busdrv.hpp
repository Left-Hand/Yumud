#ifndef __BUSDRV_HPP__
#define __BUSDRV_HPP__

#include "spi/spi.hpp"
#include "i2c/i2c.hpp"
#include "i2s/i2s.hpp"
#include <type_traits>
#include <initializer_list>

#include "i2c/i2csw.hpp"

class BusDrv{
protected:
    Bus & bus;
    uint8_t index;
    uint8_t data_size = 8;
    bool timeout;
    uint32_t wait_time;

    void configDataSize(const size_t _data_size){
        if(_data_size == data_size) return;
        else{
            data_size = _data_size;
            bus.configDataSize(_data_size);
        }
    }
public:
    BusDrv(Bus & _bus, const uint8_t & _index, const uint32_t & _wait_time = 320):bus(_bus), index(_index), wait_time(_wait_time){;}


    template<typename T>
    requires std::is_integral<T>::value
    void write(const T & data, bool discontinuous = true){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->configDataSize(sizeof(T) * 8);
            bus.write(data);
            if(discontinuous) bus.end();
            if (sizeof(T) != 1) this->configDataSize(8);
        }

    }

    void write(std::initializer_list<const uint8_t> datas, bool discontinuous = true){
        if(!bus.begin(index)){
            for(auto data_item : datas) bus.write(data_item);
            if(discontinuous) bus.end();
        }
    }

    template<typename T>
    requires std::is_integral<T>::value
    void write(const T data, const size_t len, bool discontinuous = true){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->configDataSize(sizeof(T) * 8);
            for(size_t i = 0; i < len; i++) bus.write(data);
            if (discontinuous) bus.end();
            if (sizeof(T) != 1) this->configDataSize(8);
        }
    }

    template<typename T>
    requires std::is_integral<T>::value
    void write(const T * data_ptr, const size_t len, bool discontinuous = true){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->configDataSize(sizeof(T) * 8);
            for(size_t i = 0; i < len; i++) bus.write(data_ptr[i]);
            if (discontinuous) bus.end();
            if (sizeof(T) != 1)this->configDataSize(8);
        }
    }

    template<typename T>
    requires std::is_integral<T>::value
    void read(T * data_ptr, const size_t len, const bool discontinuous = true){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++){
                uint32_t temp = 0;
                bus.read(temp, (i != len - 1));
                data_ptr[i] = temp;
            }
            if(discontinuous) bus.end();
        }
    }

    template<typename T>
    requires std::is_integral<T>::value
    void read(T & data, const bool discontinuous = true){
        if(!bus.begin(index)){
            uint32_t temp;
            bus.read(temp);
            data = temp;
            if(discontinuous) bus.end();
        }
    }

    template<typename T>
    requires std::is_integral<T>::value
    void transmit(T & datarx, T datatx, bool discontinuous = true){
        if(!bus.begin(index)){
            this->configDataSize(16);
            uint32_t ret = 0;
            bus.transfer(ret, datatx);
            datarx = ret;
            this->configDataSize(8);
            if(discontinuous) bus.end();
        }
    }
};


#endif