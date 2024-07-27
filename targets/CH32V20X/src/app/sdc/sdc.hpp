#pragma once


#include "../../drivers/device_defs.h"


#define W25QXX_DEBUG

#ifdef W25QXX_DEBUG
#define W25QXX_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define W25QXX_DEBUG(...)
#endif

class SpiDevice{

};

#define SPIDEV_CONTSRTUCTER(name)\
    name(SpiDrv & _spi_drv):spi_drv(_spi_drv){;}\
    name(SpiDrv && _spi_drv):spi_drv(_spi_drv){;}\


class W25QXX{
protected:
    SpiDrv & bus_drv;
    enum class Commands:uint8_t{
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

    struct StatusReg{
        uint8_t busy:1;
        uint8_t write_enable_latch:1;
        uint8_t block_protect_bits:3;
        uint8_t top_or_bottom_protect:1;
        uint8_t sector_or_block_protect:1;
        uint8_t __resv__:1;
    }__packed;

    {
        StatusReg statusReg;
    }

    void writeByte(const uint8_t & data){
        bus_drv.write(data);
    }

    void writeByte(const Commands & data){
        bus_drv.write((uint8_t)data);
    }

    void readByte(uint8_t & data){
        bus_drv.read(data);
    }

    void wait_for_free(){

    }

public:
    W25QXX(SpiDrv & _bus_drv):bus_drv(_bus_drv){}
    void enableWrite(const bool en = true){
        if(en){
            writeByte(Commands::WriteEnable);
        }else{
            writeByte(Commands::WriteDisable);
        }
    }

    uint8_t getDeviceManufacturer(){
        uint8_t data;
        writeByte(Commands::ReadDeviceId);
        readByte(data);
        W25QXX_DEBUG("Device Manufacturer: ", data);
        return data;
    }

    uint8_t getDeviceStorageType(){
        uint8_t data;
        uint8_t dummy;
        writeByte(Commands::ReadDeviceId);
        readByte(dummy);
        readByte(data);
        W25QXX_DEBUG("Device Storage Type: ", data);
        return data;
    }

    uint8_t getDeviceCapacity(){
        uint8_t data;
        uint8_t dummy;
        writeByte(Commands::ReadDeviceId);
        readByte(dummy);
        readByte(dummy);
        readByte(data);
        W25QXX_DEBUG("Device Capacity: ", data);
        return data;
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
        uint8_t temp = 0;
        readByte(temp);
        statusReg.data = temp;
        return statusReg.busy;
    }

    bool isWriteable(){
        writeByte(Commands::ReadStatusRegister);
        uint8_t temp = 0;
        readByte(temp);
        statusReg.data = temp;
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
        for(size_t i = 0; i < len; i++){
            writeByte(data[i]);
        }
    }

    void writeData(const uint32_t & _addr, const uint8_t * _data, const size_t & len){
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
    void readData(const uint32_t & addr, uint8_t * data, const size_t & len){
        writeByte(Commands::ReadData);
        writeByte(addr >> 16);
        writeByte(addr >> 8);
        writeByte(addr);
        for(size_t i = 0; i < len; i++){
            readByte(data[i]);
        }
    }
};


#endif