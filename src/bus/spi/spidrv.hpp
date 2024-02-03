#ifndef __SPIDRV_HPP__
#define __SPIDRV_HPP__

#include "spi2.hpp"

class SpiDrv{
protected:
    Spi & spibus;
    uint8_t index = 0;
public:
    SpiDrv(Spi & _spibus, const uint8_t & _index = 0):spibus(_spibus), index(_index){;}
    
    void write(const uint8_t & data){
        if(!spibus.begin(index)){
            spibus.write(data);
            spibus.end();
        }
    }

    void write(const uint16_t & data){
        if(!spibus.begin(index)){
            spibus.write(data);
            spibus.end();
        }
    }

    void write(uint8_t * data, const size_t & len){
        if(!spibus.begin(index)){
            spibus.write(data, len);
            spibus.end();
        }
    }

    void write(const uint8_t & data, const size_t & len){
        if(!spibus.begin(index)){
            for(size_t _ = 0; _ < len; _++) spibus.write(data);
            spibus.end();
        }
    }

    void write(uint16_t * data, const size_t & len){
        if(!spibus.begin(index)){
            spibus.write(data, len);
            spibus.end();
        }
    }

    void write(const uint16_t & data, const size_t & len){
        if(!spibus.begin(index)){
            spibus.configDataSize(16);
            for(size_t _ = 0; _ < len; _++) spibus.write(data);
            spibus.configDataSize(8);
            spibus.end();
        }
    }

};

#endif