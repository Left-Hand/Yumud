#include "Si5351C.hpp"

#define Si5351C_DEBUG

#ifdef Si5351C_DEBUG
#undef Si5351C_DEBUG
#define Si5351C_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define Si5351C_DEBUG(...)
#endif

using namespace ymd::drivers;

void Si5351C::writeReg(const uint8_t address, const uint8_t data){
    _i2c_drv.writeReg(address, data);
}
void Si5351C::writeBurst(const uint8_t address, const uint8_t * data, const size_t len){
    _i2c_drv.writeBurst(address, std::span(data, len));
}

void Si5351C::readReg(const uint8_t address, uint8_t & data){
    _i2c_drv.readReg(address, data);
}

void Si5351C::readBurst(const uint8_t address, uint8_t * data, const size_t len){
    _i2c_drv.readBurst(address, std::span(data, len));
}