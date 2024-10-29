#pragma once


#include "../../drivers/device_defs.h"
#include "../../drivers/Memory/storage.hpp"

#define W25QXX_DEBUG

#ifdef W25QXX_DEBUG
#undef W25QXX_DEBUG
#define W25QXX_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define W25QXX_DEBUG(...)
#endif

class SpiDevice{
protected:
    SpiDrv spi_drv;

    SpiDevice(const SpiDrv & _spi_drv):spi_drv(_spi_drv){;}
    SpiDevice(SpiDrv && _spi_drv):spi_drv(_spi_drv){;}
};


class W25QXX:public SpiDevice, public StoragePaged{
protected:
    enum class Command:uint8_t{
        WriteEnable = 0x06,
        WriteDisable = 0x04,
        ReadStatusRegister = 0x05,
        WriteStatusRegister = 0x01,
        ReadData = 0x03,
        FastRead = 0x0B,
        FastReadDual = 0x3B,
        PageProgram = 0x02,
        BlockErase = 0xD8,
        SectorErase = 0x20,
        ChipErase = 0xC7,
        ResetEnable = 0x66,
        ResetMemory = 0x99,
        PowerDown = 0xB9,
        ReleasePowerDown = 0xAB,
        ReadDeviceId = 0x90,
        JedecId = 0x9F
    };

    union StatusReg{
        struct {
            uint8_t busy:1;
            uint8_t write_enable_latch:1;
            uint8_t block_protect_bits:3;
            uint8_t top_or_bottom_protect:1;
            uint8_t sector_or_block_protect:1;
            uint8_t __resv__:1;
        }__packed;
        uint8_t data;
    }__packed;

    struct{
        StatusReg statusReg;
    };

    void writeByte(const uint8_t data, const bool ends = true){
        spi_drv.write(data, ends);
    }

    void writeByte(const Command data, const bool ends = true){
        spi_drv.write((uint8_t)data, ends);
    }

    void readByte(uint8_t & data, const bool ends = true){
        spi_drv.read(data, ends);
    }

    void wait_for_free(){

    }

    void entry_store() override{}
    void exit_store() override{}

    void entry_load() override{}
    void exit_load() override{}

    void loadBytes(void * data, const Address data_size, const Address loc) override{}
    void storeBytes(const void * data, const Address data_size, const Address loc) override{};
public:
    void init() override{}

    bool busy() override{return false;}
public:
    W25QXX(SpiDrv && _spi_drv):SpiDevice(_spi_drv), StoragePaged(32_MB, 256){;}

    void enableWrite(const bool en = true){
        if(en){
            writeByte(Command::WriteEnable);
        }else{
            writeByte(Command::WriteDisable);
        }
    }

    uint8_t getDeviceManufacturer(){
        uint8_t data = 1;
        writeByte(Command::ReadDeviceId, false);
        readByte(data);
        W25QXX_DEBUG("Device Manufacturer: ", data);
        return data;
    }

    uint8_t getDeviceStorageType(){
        uint8_t data;
        uint8_t dummy;
        writeByte(Command::ReadDeviceId);
        readByte(dummy);
        readByte(data);
        W25QXX_DEBUG("Device Storage Type: ", data);
        return data;
    }

    uint8_t getDeviceCapacity(){
        uint8_t data;
        uint8_t dummy;
        writeByte(Command::ReadDeviceId);
        readByte(dummy);
        readByte(dummy);
        readByte(data);
        W25QXX_DEBUG("Device Capacity: ", data);
        return data;
    }

    void eraseBlock(const Address addr){
        enableWrite();
        writeByte(Command::SectorErase);
        writeByte(addr >> 16);
        writeByte(addr >> 8);
        writeByte(addr);
    }

    void eraseSector(const Address addr){
        enableWrite();
        writeByte(Command::SectorErase);
        writeByte(addr >> 16);
        writeByte(addr >> 8);
        writeByte(addr);
    }
    void eraseChip(){
        writeByte(Command::ChipErase);
    }

    bool isIdle(){
        writeByte(Command::ReadStatusRegister);
        uint8_t temp = 0;
        readByte(temp);
        statusReg.data = temp;
        return statusReg.busy;
    }

    bool isWriteable(){
        writeByte(Command::ReadStatusRegister);
        uint8_t temp = 0;
        readByte(temp);
        statusReg.data = temp;
        return statusReg.write_enable_latch;
    }

    void writePage(const Address addr, const uint8_t * data, size_t len){
        enableWrite();
        if(len > 256){
            W25QXX_DEBUG("page too large", len);
            len = 256;
        }
        writeByte(Command::PageProgram);
        writeByte(addr >> 16);
        writeByte(addr >> 8);
        writeByte(addr);
        for(size_t i = 0; i < len; i++){
            writeByte(data[i]);
        }
    }

    void writeData(const Address _addr, const uint8_t * _data, const size_t & len){
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
    void readData(const Address addr, uint8_t * data, const size_t & len){
        writeByte(Command::ReadData);
        writeByte(addr >> 16);
        writeByte(addr >> 8);
        writeByte(addr);
        for(size_t i = 0; i < len; i++){
            readByte(data[i]);
        }
    }
};