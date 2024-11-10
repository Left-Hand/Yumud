#include "w25q16.hpp"

using namespace yumud;

void W25Q16::storeByte(const uint8_t data, const Address loc){
    this->storeBytes(&data, 1, loc);
}

void W25Q16::loadByte(uint8_t & data, const Address loc){
    this->loadBytes(&data, 1, loc);
}


void W25Q16::writePage(const void * _data, Address len, const size_t addr){
    enableWrite();

    if(len > 256){
        W25Q16_DEBUG("page too large", len);
        len = 256;
    }
    
    const uint8_t * data = reinterpret_cast<const uint8_t *>(_data);
    writeByte(Commands::PageProgram);
    writeByte(addr >> 16);
    writeByte(addr >> 8);
    writeByte(addr);
    for(size_t i = 0; i < len; i++){
        writeByte(data[i]);
    }
}

void W25Q16::storeBytes(const void * _data, const Address len, const Address _addr){
    //FIXME
    enableWrite();
    uint16_t pages = _addr / 256;
    uint32_t addr = _addr;
    const uint8_t * data = reinterpret_cast<const uint8_t *>(_data);
    for(uint16_t i = 0; i < pages; i++){
        writePage(data, 256, addr);
        addr += 256;
        data += 256;
    }
    uint8_t remains = addr % 256;
    writePage(data, remains, addr);
}

void W25Q16::loadBytes(void * data, const Address len, const Address addr){
    writeByte(Commands::ReadData);
    writeByte(addr >> 16);
    writeByte(addr >> 8);
    writeByte(addr);
    for(size_t i = 0; i < len; i++){
        readByte(reinterpret_cast<uint8_t *>(data)[i]);
    }
}


void W25Q16::entry_store(){
    //TODO
}


void W25Q16::exit_store(){
    //TODO
}

void W25Q16::entry_load(){
    //TODO
}


void W25Q16::exit_load(){
    //TODO
}



void W25Q16::enableWrite(const bool en){
    if(en){
        writeByte(Commands::WriteEnable);
    }else{
        writeByte(Commands::WriteDisable);
    }
}


uint8_t W25Q16::getDeviceManufacturer(){
    uint8_t data;
    writeByte(Commands::ReadDeviceId);
    readByte(data);
    W25Q16_DEBUG("Device Manufacturer: ", data);
    return data;
}

uint8_t W25Q16::getDeviceStorageType(){
    uint8_t data;
    uint8_t dummy;
    writeByte(Commands::ReadDeviceId);
    readByte(dummy);
    readByte(data);
    W25Q16_DEBUG("Device Storage Type: ", data);
    return data;
}


uint8_t W25Q16::getDeviceCapacity(){
    uint8_t data;
    uint8_t dummy;
    writeByte(Commands::ReadDeviceId);
    readByte(dummy);
    readByte(dummy);
    readByte(data);
    W25Q16_DEBUG("Device Capacity: ", data);
    return data;
}


void W25Q16::eraseBlock(const Address addr){
    enableWrite();
    writeByte(Commands::SectorErase);
    writeByte(addr >> 16);
    writeByte(addr >> 8);
    writeByte(addr);
}


void W25Q16::eraseSector(const Address addr){
    enableWrite();
    writeByte(Commands::SectorErase);
    writeByte(addr >> 16);
    writeByte(addr >> 8);
    writeByte(addr);
}


void W25Q16::eraseChip(){
    writeByte(Commands::ChipErase);
}


bool W25Q16::isIdle(){
    writeByte(Commands::ReadStatusRegister);
    uint8_t temp = 0;
    readByte(temp);
    statusReg = temp;
    return statusReg.busy;
}


bool W25Q16::isWriteable(){
    writeByte(Commands::ReadStatusRegister);
    uint8_t temp = 0;
    readByte(temp);
    statusReg = temp;
    return statusReg.write_enable_latch;
}