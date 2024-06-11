#ifndef __BUSDRV_HPP__
#define __BUSDRV_HPP__

#include "spi/spi.hpp"
#include "i2c/i2c.hpp"
#include "i2s/i2s.hpp"
#include <type_traits>
#include <initializer_list>

#include "i2c/i2csw.hpp"

enum class BusType{
    SpiBus,
    I2cBus,
    I2sBus
};

class BusDrv{
protected:
    Bus & bus;
    uint8_t index;
    uint32_t wait_time;
public:
    BusDrv(Bus & _bus, const uint8_t & _index, const uint32_t & _wait_time = 320):bus(_bus), index(_index), wait_time(_wait_time){;}

    void write(const uint8_t & data, bool discontinuous = true){
        if(!bus.begin(index)){
            bus.write(data);
            if(discontinuous) bus.end();
        }
    }

    void write(const uint16_t & data, bool discontinuous = true){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            bus.write(data);
            if(discontinuous) bus.end();
            bus.configDataSize(8);
        }
    }

    void write(const uint32_t & data, bool discontinuous = true){
        if(!bus.begin(index)){
            bus.write(data);
            if(discontinuous) bus.end();
        }
    }

    void write(std::initializer_list<const uint8_t> datas, bool discontinuous = true){
        if(!bus.begin(index)){
            for(auto data_item : datas) bus.write(data_item);
            if(discontinuous) bus.end();
        }
    }

    void write(const uint8_t & data, const size_t & len, bool discontinuous = true){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++) bus.write(data);
            if(discontinuous) bus.end();
        }
    }

    void write(const uint16_t & data, const size_t & len, bool discontinuous = true){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            for(size_t i = 0; i < len; i++) bus.write(data);
            if(discontinuous) bus.end();
            bus.configDataSize(8);
        }
    }

    void write(const uint8_t * data_ptr, const size_t & len, bool discontinuous = true){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++) bus.write(data_ptr[i]);
            if(discontinuous) bus.end();
        }
    }
    
    void write(const uint16_t * data_ptr, const size_t & len, bool discontinuous = true){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            for(size_t i = 0; i < len; i++) bus.write(data_ptr[i]);
            bus.configDataSize(8);
            if(discontinuous) bus.end();
        }
    }

    virtual void read(uint8_t * data_ptr, const size_t & len, const bool & discontinuous = true){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++){
                uint32_t temp = 0;
                bus.read(temp, (i != len - 1));
                data_ptr[i] = temp;
            }
            if(discontinuous) bus.end();
        }
    }

    virtual void read(uint8_t & data, const bool & discontinuous = true){
        if(!bus.begin(index)){
            uint32_t temp;
            bus.read(temp);
            data = temp;
            if(discontinuous) bus.end();
        }
    }

    virtual void read(uint16_t & data){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            uint32_t temp;
            bus.read(temp);
            data = temp;
            bus.configDataSize(8);
            bus.end();
        }
    }

    void transmit(uint8_t & datarx, const uint8_t & datatx, bool discontinuous = true){
        if(!bus.begin(index)){
            uint32_t ret = 0;
            bus.transfer(ret, datatx);
            datarx = ret;
            if(discontinuous) bus.end();
        }
    }

    void transmit(uint16_t & datarx, const uint16_t & datatx, bool discontinuous = true){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            uint32_t ret = 0;
            bus.transfer(ret, datatx);
            datarx = ret;
            bus.configDataSize(8);
            if(discontinuous) bus.end();
        }
    }
    virtual BusType getBusType() = 0;
};

class SpiDrv{
protected:
    Spi & bus;
    uint8_t index;
    uint8_t data_size = 8;
    uint32_t wait_time;


public:
    SpiDrv(Spi & _bus, const uint8_t & _index, const uint32_t & _wait_time = 320):bus(_bus), index(_index), wait_time(_wait_time){;}

    void write(const uint8_t & data, bool discontinuous = true){
        if(!bus.begin(index)){
            bus.write(data);
            if(discontinuous) bus.end();
        }
    }

    __fast_inline void configDatasize(const uint8_t & _data_size){
        if(_data_size == data_size) return;
        else{
            data_size = _data_size;
            bus.configDataSize(_data_size);
        }
    }

    void write(const uint16_t & data, bool discontinuous = true){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            bus.write(data);
            if(discontinuous) bus.end();
            bus.configDataSize(8);
        }
    }

    void write(const uint32_t & data, bool discontinuous = true){
        if(!bus.begin(index)){
            bus.write(data);
            if(discontinuous) bus.end();
        }
    }

    void write(std::initializer_list<const uint8_t> datas, bool discontinuous = true){
        if(!bus.begin(index)){
            for(auto data_item : datas) bus.write(data_item);
            if(discontinuous) bus.end();
        }
    }

    void write(const uint8_t & data, const size_t & len, bool discontinuous = true){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++) bus.write(data);
            if(discontinuous) bus.end();
        }
    }

    void write(const uint16_t & data, const size_t & len, bool discontinuous = true){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            for(size_t i = 0; i < len; i++) bus.write(data);
            if(discontinuous) bus.end();
            bus.configDataSize(8);
        }
    }

    void write(const uint8_t * data_ptr, const size_t & len, bool discontinuous = true){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++) bus.write(data_ptr[i]);
            if(discontinuous) bus.end();
        }
    }

    void write(const uint16_t * data_ptr, const size_t & len, bool discontinuous = true){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            for(size_t i = 0; i < len; i++) bus.write(data_ptr[i]);
            bus.configDataSize(8);
            if(discontinuous) bus.end();
        }
    }

    void read(uint8_t * data_ptr, const size_t & len, const bool & discontinuous = true){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++){
                uint32_t temp = 0;
                bus.read(temp, (i != len - 1));
                data_ptr[i] = temp;
            }
            if(discontinuous) bus.end();
        }
    }

    void read(uint8_t & data, const bool & discontinuous = true){
        if(!bus.begin(index)){
            uint32_t temp;
            bus.read(temp);
            data = temp;
            if(discontinuous) bus.end();
        }
    }

    void read(uint16_t & data){
        if(!bus.begin(index)){
            bus.configDataSize(16);
            uint32_t temp;
            bus.read(temp);
            data = temp;
            bus.configDataSize(8);
            bus.end();
        }
    }

    void transmit(uint16_t & datarx, const uint16_t & datatx, bool discontinuous = true){
        if(!bus.begin(index)){
            configDatasize(16);
            uint32_t ret = 0;
            bus.transfer(ret, datatx);
            datarx = ret;
            if(discontinuous) bus.end();
        }
    }

};

#endif