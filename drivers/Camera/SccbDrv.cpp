#include "SccbDrv.hpp"

using namespace yumud;

void SccbDrv::writeReg(const uint8_t reg_address, const uint16_t reg_data){
    if(bus.begin(index) != Bus::ErrorType::OK){
        bus.write(reg_address);
        bus.write((uint8_t)(reg_data >> 8));
        bus.begin(index);
        bus.write(0xF0);
        bus.write((uint8_t)reg_data);
        bus.end();
    }
}

void SccbDrv::readReg(const uint8_t reg_address, uint16_t & reg_data){
    if(bus.begin(index) != Bus::ErrorType::OK){
        uint32_t data_l, data_h;
        bus.write(reg_address);
        bus.begin(index | 0x01);
        bus.read(data_h, false);

        bus.begin(index);
        bus.write(0xF0);
        bus.begin(index | 0x01);
        bus.read(data_l, false);
        bus.end();

        reg_data = ((uint8_t)data_h << 8) | (uint8_t)data_l;
    }
}