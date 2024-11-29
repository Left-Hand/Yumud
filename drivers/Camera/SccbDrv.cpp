#include "SccbDrv.hpp"

using namespace ymd;

void SccbDrv::writeReg(const uint8_t reg_address, const uint16_t reg_data){
    if(bus_.begin(index_) != Bus::ErrorType::OK){
        bus_.write(reg_address);
        bus_.write((uint8_t)(reg_data >> 8));
        bus_.begin(index_);
        bus_.write(0xF0);
        bus_.write((uint8_t)reg_data);
        bus_.end();
    }
}

void SccbDrv::readReg(const uint8_t reg_address, uint16_t & reg_data){
    if(bus_.begin(index_) != Bus::ErrorType::OK){
        uint32_t data_l, data_h;
        bus_.write(reg_address);
        bus_.begin(index_ | 0x01);
        bus_.read(data_h, false);

        bus_.begin(index_);
        bus_.write(0xF0);
        bus_.begin(index_ | 0x01);
        bus_.read(data_l, false);
        bus_.end();

        reg_data = ((uint8_t)data_h << 8) | (uint8_t)data_l;
    }
}