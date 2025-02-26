#include "SccbDrv.hpp"

using namespace ymd::hal;

void SccbDrv::writeReg(const uint8_t reg_address, const uint16_t reg_data){
    if(bus_.begin(index_).ok()){
        
        //#region 写入地址字节和第一个字节
        bus_.write(reg_address);
        bus_.write((uint8_t)(reg_data >> 8));
        //#endregion
    
        //#region 写入第二个字节
        bus_.begin(index_);
        bus_.write(0xF0);
        bus_.write((uint8_t)reg_data);
        //#endregion
        bus_.end();
    }
}

void SccbDrv::readReg(const uint8_t reg_address, uint16_t & reg_data){
    if(bus_.begin(index_).ok()){
        uint32_t data_l, data_h;

        // 写入地址字节
        bus_.write(reg_address);

        // 写入第一个字节        
        bus_.begin(index_ | 0x01);
        bus_.read(data_h, NACK);

        // 写入第二个字节
        bus_.begin(index_);
        bus_.write(0xF0);

        bus_.begin(index_ | 0x01);
        bus_.read(data_l, NACK);
        
        bus_.end();

        reg_data = ((uint8_t)data_h << 8) | (uint8_t)data_l;
    }
}