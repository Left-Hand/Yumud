#ifndef __BUSDRV_HPP__
#define __BUSDRV_HPP__

#include "spi/spi.hpp"
#include "i2c/i2c.hpp"
#include "i2s/i2s.hpp"
#include <type_traits>
#include <initializer_list>

#include "uart/uart1.hpp"
#include "i2c/i2csw.hpp"

enum class BusType{
    SpiBus,
    I2cBus,
    I2sBus
};

class BusDrv{
protected:
    Bus & bus;
    uint8_t index = 0;
    BusType bus_type = BusType::SpiBus;

public:
    BusDrv(Bus & _bus, const uint8_t & _index):bus(_bus), index(_index){;}

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

    virtual void read(uint8_t * data_ptr, const size_t & len){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++){
                uint32_t temp = 0;
                bus.read(temp, (i != len - 1));
                data_ptr[i] = temp;
            }
            bus.end();
        }
    }

    virtual void read(uint8_t & data){
        if(!bus.begin(index)){
            uint32_t temp;
            bus.read(temp);
            data = temp;
            bus.end();
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

#endif