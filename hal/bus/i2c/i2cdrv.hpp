#ifndef __I2C_DRV_HPP__
#define __I2C_DRV_HPP__

#include "i2c.hpp"
#include "sys/debug/debug_inc.h"
#include "hal/bus/busdrv.hpp"

#include <type_traits>
#include <initializer_list>

class I2cDrv : public BusDrv<I2c> {
protected:
    using BusDrv<I2c>::index;
    using BusDrv<I2c>::bus;

public:
    I2cDrv(I2c & _bus, const uint8_t & _index, const uint32_t & _wait_time = 320):
        BusDrv<I2c>(_bus, _index, _wait_time){};

    template<typename T>
    requires (std::is_same_v<T, uint16_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t>) && (!std::is_same_v<T, int>)
    void writePool(const uint8_t reg_address, const T * data_ptr, const size_t length, const bool msb = true){
        constexpr size_t size = sizeof(T);
        if(length == 0) return;
        size_t bytes = length * size;
        if(!bus.begin(index)){
            bus.write(reg_address);

            for(size_t i = 0; i < bytes; i += size){
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

    template<typename T>
    void readPool(const uint8_t reg_address, T * data_ptr, const size_t length, const bool msb = true){
        constexpr size_t size = sizeof(T);
        if(length == 0) return;
        size_t bytes = length * size;
        if(!bus.begin(index)){
            bus.write(reg_address);
            if(!bus.begin(index | 0x01)){
                for(size_t i = 0; i < bytes; i += size){
                    if(msb){
                        for(size_t j = size; j > 0; j--){
                            uint32_t temp = 0;
                            bus.read(temp, !((j == 1) && (i == bytes - size)));
                            data_ptr[j-1 + i] = temp;
                        }
                    }else{
                        for(size_t j = 0; j < size; j++){
                            uint32_t temp = 0;
                            bus.read(temp, (i + j != bytes - 1));
                            data_ptr[j + i] = temp;
                        }
                    }
                }
            }
            bus.end();
        }
    }

    template<typename T>
    void readReg(const uint8_t reg_address,T & reg, bool msb = true){
        readPool(reg_address, &reg, 1, msb);
    }

    void readReg(const uint8_t reg_address, uint8_t & reg_data){
        if(!bus.begin(index)){
            bus.write(reg_address);
            bus.begin(index | 0x01);
            uint32_t temp;
            bus.read(temp, false);
            reg_data = temp;
            bus.end();
        }
    }

    template<typename T>
    void writeReg(const uint8_t reg_address, const T reg_data, bool msb = true){
        writePool(reg_address, &reg_data, 1, msb);
    }


    void writeReg(const uint8_t reg_address,  const uint8_t reg_data){
        if(!bus.begin(index)){
            bus.write(reg_address);
            bus.write(reg_data);
            bus.end();
        }
    }
};


#endif