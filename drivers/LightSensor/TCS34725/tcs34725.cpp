#include "tcs34725.hpp"

using namespace yumud::drivers;

#ifdef TCS34725_DEBUG
#undef TCS34725_DEBUG
#define TCS34725_DEBUG(...) DEBUG_PRINT(__VA_ARGS__)
#else
#define TCS34725_DEBUG(...)
#endif

void TCS34725::requestRegData(const RegAddress regAddress, uint16_t * data_ptr, const size_t len){
    uint8_t address = convRegAddress(regAddress);
    TCS34725_DEBUG("address", address)

    i2c_drv_.readMulti(address, data_ptr, len, LSB);
}


void TCS34725::getCRGB(real_t & c, real_t & r, real_t & g, real_t & b){
    s16_to_uni(data[0], c);
    s16_to_uni(data[1], r);
    s16_to_uni(data[2], g);
    s16_to_uni(data[3], b);
}

void TCS34725::update(){
    requestRegData(RegAddress::ClearData, data, 4);
}


void TCS34725::setIntPersistence(const uint8_t times){
    if(times >= 5){
        uint8_t value = 0b0100 + (times / 5) - 1;
        intPersistenceReg = value;
    }else{
        intPersistenceReg = (uint8_t)MIN(times, 3);
    }

    writeReg(RegAddress::Integration, integrationReg);
}
