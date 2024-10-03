#include "w25q16.hpp"


void W25Q16::storeBytes(const uint8_t data, const Address loc){}
void W25Q16::loadBytes(uint8_t & data, const Address loc){}
void W25Q16::storeBytes(const void * data, const Address data_size, const Address loc){}
void W25Q16::loadBytes(void * data, const Address data_size, const Address loc){}
void W25Q16::entry_store(){}
void W25Q16::exit_store(){}
void W25Q16::entry_load(){}
void W25Q16::exit_load(){}



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

void W25Q16::eraseBlock(const uint32_t addr){
    enableWrite();
    writeByte(Commands::SectorErase);
    writeByte(addr >> 16);
    writeByte(addr >> 8);
    writeByte(addr);
}

void W25Q16::eraseSector(const uint32_t addr){
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
    statusReg.data = temp;
    return statusReg.busy;
}

bool W25Q16::isWriteable(){
    writeByte(Commands::ReadStatusRegister);
    uint8_t temp = 0;
    readByte(temp);
    statusReg.data = temp;
    return statusReg.write_enable_latch;
}