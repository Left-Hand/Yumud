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


struct WRFormat:public Reg16{
    uint16_t addr:12;
    uint16_t type:4;
};



void MT6835::writeReg(const RegAddress addr, const uint8_t data){

    WRFormat format = {
        .addr = addr,
        .type = 0b0110
    };

    spi_drv_.writeSingle<uint16_t>(format, CONT);
    spi_drv_.writeSingle<uint8_t>(data);
}


void MT6835::readReg(const RegAddress addr, uint8_t & data){

    WRFormat format = {
        .addr = addr,
        .type = 0b0011
    };

    spi_drv_.writeSingle<uint16_t>(format, CONT);
    spi_drv_.readSingle<uint8_t>(data);
}