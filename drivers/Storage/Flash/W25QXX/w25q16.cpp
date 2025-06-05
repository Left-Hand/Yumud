#include "w25q16.hpp"

#include "core/debug/debug.hpp"

#ifdef W25Q16_DEBUG
#define W25Q16_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define W25Q16_DEBUG(...)
#endif

#if 0
using namespace ymd;
void W25Q16::write_page(const size_t addr, const void * _data, Address len){
    enable_write();

    if(len > 256){
        W25Q16_DEBUG("page too large", len);
        len = 256;
    }
    
    const uint8_t * data = reinterpret_cast<const uint8_t *>(_data);
    write_byte(Commands::PageProgram);
    write_byte(addr >> 16);
    write_byte(addr >> 8);
    write_byte(addr);
    for(size_t i = 0; i < len; i++){
        write_byte(data[i]);
    }
}

void W25Q16::store_bytes(const Address _addr, const void * _data, const Address len){
    //FIXME
    enable_write();
    uint16_t pages = _addr / 256;
    uint32_t addr = _addr;
    const uint8_t * data = reinterpret_cast<const uint8_t *>(_data);
    for(uint16_t i = 0; i < pages; i++){
        write_page(addr, data, 256);
        addr += 256;
        data += 256;
    }
    uint8_t remains = addr % 256;
    write_page(addr, data, remains);
}

void W25Q16::load_bytes( const Address addr, void * data, const Address len){
    write_byte(Commands::ReadData);
    write_byte(addr >> 16);
    write_byte(addr >> 8);
    write_byte(addr);
    for(size_t i = 0; i < len; i++){
        read_byte(reinterpret_cast<uint8_t *>(data)[i]);
    }
}


void W25Q16::entry_store(){
    //TODO
    TODO();
}


void W25Q16::exit_store(){
    //TODO
    TODO();
}

void W25Q16::entry_load(){
    //TODO
    TODO();
}


void W25Q16::exit_load(){
    //TODO
    TODO();
}



void W25Q16::enable_write(const Enable en){
    if(en){
        write_byte(Commands::WriteEnable);
    }else{
        write_byte(Commands::WriteDisable);
    }
}


uint8_t W25Q16::get_device_manufacturer(){
    uint8_t data;
    write_byte(Commands::ReadDeviceId);
    read_byte(data);
    W25Q16_DEBUG("Device Manufacturer: ", data);
    return data;
}

uint8_t W25Q16::get_device_storage_type(){
    uint8_t data;
    uint8_t dummy;
    write_byte(Commands::ReadDeviceId);
    read_byte(dummy);
    read_byte(data);
    W25Q16_DEBUG("Device Storage Type: ", data);
    return data;
}


uint8_t W25Q16::get_device_capacity(){
    uint8_t data;
    uint8_t dummy;
    write_byte(Commands::ReadDeviceId);
    read_byte(dummy);
    read_byte(dummy);
    read_byte(data);
    W25Q16_DEBUG("Device Capacity: ", data);
    return data;
}


void W25Q16::erase_block(const Address addr){
    enable_write();
    write_byte(Commands::SectorErase);
    write_byte(addr >> 16);
    write_byte(addr >> 8);
    write_byte(addr);
}


void W25Q16::erase_sector(const Address addr){
    enable_write();
    write_byte(Commands::SectorErase);
    write_byte(addr >> 16);
    write_byte(addr >> 8);
    write_byte(addr);
}


void W25Q16::erase_chip(){
    write_byte(Commands::ChipErase);
}


bool W25Q16::is_idle(){
    write_byte(Commands::ReadStatusRegister);
    uint8_t temp = 0;
    read_byte(temp);
    statusReg = temp;
    return statusReg.busy;
}


bool W25Q16::is_writeable(){
    write_byte(Commands::ReadStatusRegister);
    uint8_t temp = 0;
    read_byte(temp);
    statusReg = temp;
    return statusReg.write_enable_latch;
}

#endif