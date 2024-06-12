#ifndef __I2C_DRV_HPP__
#define __I2C_DRV_HPP__

#include "i2c.hpp"
#include "../busdrv.hpp"
#include <type_traits>
#include <initializer_list>

class I2cDrv:public BusDrv{
protected:
    using BusDrv::write;
    using BusDrv::read;
    using BusDrv::transmit;
public:
    I2cDrv(I2c & _bus, const uint8_t _index = 0):BusDrv(_bus, _index){;}

    void writePool(const uint8_t reg_address, const uint8_t * data_ptr, const size_t size, const size_t length, const bool msb = true){
        if(length == 0) return;
        if(!bus.begin(index)){
            bus.write(reg_address);

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

    void readPool(const uint8_t reg_address, uint8_t * data_ptr, const size_t size, const size_t length, const bool msb = true){
        if(length == 0) return;
        if(!bus.begin(index)){
            bus.write(reg_address);
            if(!bus.begin(index | 0x01)){
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
                            bus.read(temp, (i + j != length - 1));
                            data_ptr[j + i] = temp;
                        }
                    }
                }
            }
            bus.end();
        }
    }

    void readReg(const uint8_t reg_address,uint16_t & reg, bool msb = true){
        uint8_t buf[2] = {0};
        readPool(reg_address, buf, 2, 2, msb);
        reg = buf[1] << 8 | buf[0];
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

    void writeReg(const uint8_t reg_address,  const uint16_t reg_data, bool msb = true){
        writePool(reg_address, (uint8_t *)&reg_data, 2, 2, msb);
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