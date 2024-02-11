#ifndef __W25QXX_HPP__

#define __W25Qxx_HPP__

#include "device_defs.h"
#include "real.hpp"

#ifndef W25QXX_DEBUG
#define W25QXX_DEBUG(...) DEBUG_LOG(...)
#endif

#ifndef REG8_BEGIN
#define REG8_BEGIN union{struct{
#endif

#ifndef REG8_END
#define REG8_END };uint8_t data;};
#endif

class W25QXX{
protected:
    enum class Commands:uint8_t{
        WriteEnable = 0x06,
        WriteDisable = 0x04,
        ReadStatusRegister = 0x05,
        WriteStatusRegister = 0x01,
        ReadData = 0x03,
        PageProgram = 0x02,
        SectorErase = 0x20,
        ChipErase = 0xC7,
        ResetEnable = 0x66,
        ResetMemory = 0x99,
        PowerDown = 0xB9,
        ReleasePowerDown = 0xAB,
        ReadDeviceId = 0x90,
        JedecId = 0x9F
    };

    struct Reg8{};
    struct StatusReg:public Reg8{
        REG8_BEGIN
        uint8_t busy:1;
        uint8_t write_enable_latch:1;
        uint8_t block_protect_bits:3;
        uint8_t top_or_bottom_protect:1;
        uint8_t sector_or_block_protect:1;
        uint8_t __resv__:1;
    };
    StatusReg statusReg;

    void writeByte(const uint8_t & data){
        bus_drv.write(data);
    }

    void readByte(uint8_t & data){
        bus_drv.read(data);
    }

public:
    void enableWrite(const bool & en){
        if(en){
            writeByte(Commands::WriteEnable);
        }else{
            writeByte(Commands::WriteDisable);
        }
    }

    void getDeviceManufacturer(){
        uint8_t data;
        writeByte(Commands::ReadDeviceId);
        readByte(data);
        W25QXX_DEBUG("Device Manufacturer: ", data);
    }

    void getDeviceStorageType(){
        uint8_t data;
        uint8_t dummy;
        writeByte(Commands::ReadDeviceId);
        readByte(dummy);
        readByte(data);
        W25QXX_DEBUG("Device Storage Type: ", data);
    }

    void getDeviceCapacity(){
        uint8_t data;
        uint8_t dummy;
        writeByte(Commands::ReadDeviceId);
        readByte(dummy);
        readByte(dummy);
        readByte(data);
        W25QXX_DEBUG("Device Capacity: ", data);
    }

    void eraseBlock(const uint32_t & addr){
        enableWrite();
        writeByte(Commands::SectorErase);
        writeByte(addr >> 16);
        writeByte(addr >> 8);
        writeByte(addr);
    }

    void eraseSector(const uint32_t & addr){
        enableWrite();
        writeByte(Commands::SectorErase);
        writeByte(addr >> 16);
        writeByte(addr >> 8);
        writeByte(addr);
    }
    void eraseChip(){
        writeByte(Commands::ChipErase);
    }

    bool isIdle(){
        writeByte(Commands::ReadStatusRegister);
        readByte(*static_cast<uint8_t *>(&statusReg));
        return statusReg.busy;
    }

    bool isWriteable(){
        writeByte(Commands::ReadStatusRegister);
        readByte(*static_cast<uint8_t *>(&statusReg));
        return statusReg.write_enable_latch;
    }

    void writePage(const uint32_t & addr, const uint8_t * data, size_t len){
        enableWrite();
        if(len > 256){
            W25QXX_DEBUG("page too large", len);
            len = 256;
        }
        writeByte(Commands::PageProgram);
        writeByte(addr >> 16);
        writeByte(addr >> 8);
        writeByte(addr);
        for(size_t i = 0; i < size; i++){
            writeByte(data[i]);
        }
    }

    void writeData(const uint32_t & _addr, const uint8_t * _data, coonst size_t & len){
        enableWrite();
        uint16_t pages = addr / 256;
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
    void readData(const uint32_t & addr, const uint8_t * data, coonst size_t & len){
        writeByte(Commands::ReadData);
        writeByte(addr >> 16);
        writeByte(addr >> 8);
        writeByte(addr);
        for(size_t i = 0; i < len; i++){
            readByte(data[i]);
        }
    }
}
#ifdef W25QXX_DEBUG
#undef W25QXX_DEBUG
#endif
#endif