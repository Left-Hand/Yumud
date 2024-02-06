#ifndef __BUSDRV_HPP__
#define __BUSDRV_HPP__

#include "spi/spi.hpp"
#include "i2c/i2c.hpp"
#include <type_traits>
#include <initializer_list>

#include "uart/uart1.hpp"
#include "i2c/i2csw.hpp"
enum class BusType{
    SpiBus,
    I2cBus
};

class BusDrv{
protected:
    Bus & bus;
    uint8_t index = 0;
    BusType bus_type = BusType::SpiBus;

    uint8_t getIndex(bool isread){return index | (bool)(isI2cBus() && isread);}
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
            bus.configDataSize(16);
            for(size_t i = 0; i < len; i++) bus.write(data);
            if(discont) bus.end();
            bus.configDataSize(8);
        }
    }

    void write(uint8_t * data_ptr, const size_t & len, bool discont = true){
        if(!bus.begin(index)){
            for(size_t i = 0; i < len; i++) bus.write(data_ptr[i]);
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
        if(!bus.begin(getIndex(true))){
            for(size_t i = 0; i < len; i++){
                uint32_t temp = 0;
                bus.read(temp, (i != len - 1));
                data_ptr[i] = temp;
            }
            bus.end();
        }
    }

    void read(uint8_t & data){
        if(!bus.begin(getIndex(true))){
            uint32_t temp;
            bus.read(temp);
            data = temp;
            bus.end();
        }
    }

    void writePool(const uint8_t & reg_address, uint8_t * data_ptr, const size_t & size, const size_t & length, const bool msb = true){
        if(!bus.begin(getIndex(false))){
            bus.write(reg_address);
            // bus.begin(getIndex(false));

            for(size_t i = 0; i < length; i += size){
                if(msb){
                    for(size_t j = size; j > 0; j--){
                        bus.write(data_ptr[j-1 + i]);
                    }
                }else{
                    for(size_t j = 0; j < size; j++){
                        bus.write(data_ptr[j + i]);
                    }
                }
            }

            bus.end();
        }
    }

    void readPool(const uint8_t & reg_address, uint8_t * data_ptr, const size_t & size, const size_t & length, const bool msb = true){
        if(!bus.begin(getIndex(false))){
            bus.write(reg_address);
            if(!bus.begin(getIndex(true))){
                for(size_t i = 0; i < length; i += size){
                    if(msb){
                        for(size_t j = size; j > 0; j--){
                            uint32_t temp = 0;
                            bus.read(temp, !((j == 1) && (i == length - size)));
                            data_ptr[j-1 + i] = temp;
                        }
                    }else{
                        for(size_t j = 0; j < size; j++){
                            uint32_t temp = 0;
                            bus.read(temp, true);
                            data_ptr[j + i] = temp;
                        }
                    }
                }
            }
            bus.end();
        }
    }

    void readReg(const uint8_t & reg_address, uint16_t & reg, bool msb = true){
        uint8_t buf[2] = {0};
        readPool(reg_address, buf, 2, 2, msb);
        reg = buf[1] << 8 | buf[0];
    }

    void readReg(const uint8_t & reg_address, uint8_t & reg_data){
        // uart1.println(bus.)
        if(!bus.begin(getIndex(false))){
            bus.write(reg_address);
            // if(bus.begin(getIndex(true)) == Bus::ErrorType::OCCUPIED){
                // uart1.println(bus.ownbywho(), getIndex(true));
                // bus.write(0x99);
            // }
            bus.begin(getIndex(true));
            uint32_t temp;
            bus.read(temp, false);
            reg_data = temp;
            bus.end();
        }
    }

    void writeReg(const uint8_t & reg_address,  const uint16_t & reg_data, bool msb = true){
        writePool(reg_address, (uint8_t *)&reg_data, 2, 2, msb);
    }

    void writeReg(const uint8_t & reg_address,  const uint8_t & reg_data){
        if(!bus.begin(getIndex(false))){
            bus.write(reg_address);
            bus.write(reg_data);
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