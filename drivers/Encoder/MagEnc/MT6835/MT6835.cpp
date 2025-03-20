#include "MT6835.hpp"


using namespace ymd::drivers;
using namespace ymd;



void MT6835::init() {

}

uint16_t MT6835::getPositionData(){
    return 0;
}

void MT6835::update() {

}


struct WRFormat{
    uint16_t addr:12;
    uint16_t type:4;

    operator uint16_t & (){return *reinterpret_cast<uint16_t *>(this);}
    operator uint16_t () const {return *reinterpret_cast<const uint16_t *>(this);}
};



BusError MT6835::write_reg(const RegAddress addr, const uint8_t data){

    WRFormat format = {
        .addr = addr,
        .type = 0b0110
    };

    spi_drv_.writeSingle<uint16_t>(format, CONT).unwrap();
    return spi_drv_.writeSingle<uint8_t>(data);
}


BusError MT6835::read_reg(const RegAddress addr, uint8_t & data){

    WRFormat format = {
        .addr = addr,
        .type = 0b0011
    };

    spi_drv_.writeSingle<uint16_t>(format, CONT).unwrap();
    return spi_drv_.readSingle<uint8_t>(data);
}