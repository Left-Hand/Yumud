#ifndef __PCF8574_HPP__

#define __PCF8574_HPP__

#include "../bus/busdrv.hpp"

class PCF8574{
protected:
    BusDrv & busdrv;

    uint8_t data;
public:
    PCF8574(BusDrv & _busdrv):busdrv(_busdrv){;}
    void write(const uint8_t & _data){
        data = _data;
        busdrv.write(data);
    }

    uint8_t read(){
        uint8_t ret = 0;
        busdrv.read(ret);
        return ret;
    }

    void writeBit(const uint16_t & index, const bool & value){
        if(value) data |= (1 << index);
        else data &= (~(1 << index));
        busdrv.write(data);
    }

    bool readBit(const uint16_t & index){
        return (read() & (1 << index));
    }
};

#endif