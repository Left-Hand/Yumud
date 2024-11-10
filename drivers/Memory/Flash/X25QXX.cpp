#include "X25QXX.hpp"

using namespace yumud::drivers;

void X25QXX::writePage(const Address addr, const uint8_t * data, size_t len){
    enableWrite();
    if(len > 256){
        X25QXX_DEBUG("page too large", len);
        len = 256;
    }
    writeByte(Command::PageProgram, CONT);
    writeAddr(addr, CONT);
    for(size_t i = 0; i < len; i++){
        writeByte(data[i], CONT);
    }
}

void X25QXX::writeData(const Address _addr, const uint8_t * _data, const size_t len){
    enableWrite();
    uint16_t pages = _addr / 256;
    uint32_t addr = _addr;
    uint8_t * data = (uint8_t *)_data;
    for(uint16_t i = 0; i < pages; i++){
        writePage(addr, data, 256);
        addr += 256;
        data += 256;
    }
    uint8_t remains = addr % 256;
    writePage(addr, data, remains);
}

void X25QXX::readData(const Address addr, uint8_t * data, const size_t len){
    writeByte(Command::ReadData);
    writeByte(addr >> 16);
    writeByte(addr >> 8);
    writeByte(addr);
    for(size_t i = 0; i < len; i++){
        readByte(data[i]);
    }
}