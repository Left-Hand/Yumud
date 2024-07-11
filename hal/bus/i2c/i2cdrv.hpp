#pragma once

#include "i2c.hpp"
#include "sys/debug/debug_inc.h"
#include "hal/bus/busdrv.hpp"

#include <type_traits>
#include <concepts>
#include <initializer_list>

namespace I2cUtils{
    template <typename T>
    concept ValidAddress = std::integral<T> && (sizeof(T) <= 2) && std::is_unsigned_v<T>;

    template <typename T>
    concept ValidData = std::integral<T> && (sizeof(T) <= 4);

    template <typename U, typename T>
    concept ValidTypes = ValidAddress<U> && ValidData<T>;
}

class I2cDrv : public BusDrv<I2c> {
private:

protected:
    using BusDrv<I2c>::index;
    using BusDrv<I2c>::bus;

    // using I2cUtils::ValidData;
private:
    template<typename T>
    requires I2cUtils::ValidAddress<T>
    void writeRegAddress(const T reg_address, const Endian endian = MSB){
        if(bool(endian)){
            for(int i = sizeof(T) - 1; i >= 0; i--){
                bus.write(((uint8_t *)&reg_address)[i]);
            }
        }else{
            for(int i = 0; i < (int)sizeof(T); i++){
                bus.write(((uint8_t *)&reg_address)[i]);
            }
        }
    }

    template<typename U, typename T>
    requires I2cUtils::ValidTypes<U, T>
    void writePool_impl(const U reg_address, const T * data_ptr, const size_t length, const Endian msb = MSB){
        constexpr size_t size = sizeof(T);

        if constexpr(size == 0)   return;
        if(length == 0) return;

        size_t bytes = length * size;
        uint8_t * u8_ptr = (uint8_t *)data_ptr;

        if(!bus.begin(index)){
            writeRegAddress(reg_address);

            for(size_t i = 0; i < bytes; i += size){
                if(msb){
                    for(size_t j = size; j > 0; j--){
                        bus.write(u8_ptr[j-1 + i]);
                    }
                }else{
                    for(size_t j = 0; j < size; j++){
                        bus.write(u8_ptr[j + i]);
                    }
                }
            }

            bus.end();
        }
    }

    template<typename U, typename T>
    requires I2cUtils::ValidTypes<U, T>
    void readPool_impl(const U reg_address, T * data_ptr, const size_t length, const Endian msb = MSB){
        if(length == 0) return;
        constexpr size_t size = sizeof(T);
        size_t bytes = length * size;
        uint8_t * u8_ptr = (uint8_t *)data_ptr;
    
        if(!bus.begin(index)){
            writeRegAddress(reg_address);
            if(!bus.begin(index | 0x01)){
                for(size_t i = 0; i < bytes; i += size){
                    if(msb){
                        for(size_t j = size; j > 0; j--){
                            uint32_t temp = 0;
                            bus.read(temp, !((j == 1) && (i == bytes - size)));
                            u8_ptr[j-1 + i] = temp;
                        }
                    }else{
                        for(size_t j = 0; j < size; j++){
                            uint32_t temp = 0;
                            bus.read(temp, (i + j != bytes - 1));
                            u8_ptr[j + i] = temp;
                        }
                    }
                }
            }
            bus.end();
        }
    }

public:
    I2cDrv(I2c & _bus, const uint8_t & _index, const uint32_t & _wait_time = 320):
        BusDrv<I2c>(_bus, _index, _wait_time){};


    template<typename U, typename T>
    requires I2cUtils::ValidTypes<U, T>
    void writePool(const U reg_address, const T * data_ptr, const size_t length, const Endian msb = MSB){
        writePool_impl<U, T>(reg_address, data_ptr, length, msb);
    }


    template<typename U, typename T>
    requires I2cUtils::ValidTypes<U, T>
    void writeReg(const U reg_address, const T reg_data, Endian msb = MSB){
        writePool<U, T>(reg_address, &reg_data, 1, msb);
    }

    template<typename U, typename T>
    requires I2cUtils::ValidTypes<U, T>
    void readPool(const U reg_address, T * data_ptr, const size_t length, const Endian msb = MSB){
        readPool_impl<U, T>(reg_address, data_ptr, length, msb);
    }

    template<typename U, typename T>
    requires I2cUtils::ValidTypes<U, T>
    void readReg(const U reg_address, T & reg_data, Endian msb = MSB){
        readPool<U, T>(reg_address, &reg_data, 1, msb);
    }

    template<typename U, typename T>
    requires I2cUtils::ValidTypes<U, T>
    T readReg(const U reg_address, Endian msb = MSB){
        T reg_data;
        readPool<U, T>(reg_address, &reg_data, 1, msb);
        return reg_data;
    }
};