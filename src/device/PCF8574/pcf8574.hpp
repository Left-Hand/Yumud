#ifndef __PCF8574_HPP__

#define __PCF8574_HPP__

#include "../bus/busdrv.hpp"

class PCF8574{
protected:
    I2cDrv & bus_drv;

    uint8_t data;
public:
    PCF8574(I2cDrv & _bus_drv):bus_drv(_bus_drv){;}
    void write(const uint8_t & _data){
        data = _data;
        bus_drv.write(data);
    }

    uint8_t read(){
        uint8_t ret = 0;
        bus_drv.read(ret);
        return ret;
    }

    void writeBit(const uint16_t & index, const bool & value){
        if(value) data |= (1 << index);
        else data &= (~(1 << index));
        bus_drv.write(data);
    }

    bool readBit(const uint16_t & index){
        return (read() & (1 << index));
    }
};

#endif