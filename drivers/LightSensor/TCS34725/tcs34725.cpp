#include "tcs34725.hpp"

#ifdef TCS34725_DEBUG
#undef TCS34725_DEBUG
#define TCS34725_DEBUG(...) DEBUG_PRINT(__VA_ARGS__)
#else
#define TCS34725_DEBUG(...)
#endif

void TCS34725::requestRegData(const RegAddress & regAddress, uint16_t * data_ptr, const size_t len){
    uint8_t address = convRegAddress(regAddress);
    TCS34725_DEBUG("address", address)

    bus_drv.readPool(address, data_ptr, len, false);
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

TCS34725::operator RGB888(){
    return RGB888{
        uint8_t(data[1] >> 8), uint8_t(data[2] >> 8), uint8_t(data[3] >> 8)
    };
}
