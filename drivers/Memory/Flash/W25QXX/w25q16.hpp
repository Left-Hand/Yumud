#pragma once

#include "drivers/device_defs.h"
#include "concept/memory.hpp"

namespace ymd{

#ifdef W25Q16_DEBUG
#define W25Q16_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define W25Q16_DEBUG(...)
#endif

class W25Q16:public StoragePaged{
protected:
    hal::SpiDrv spi_drv;

    scexpr size_t _m_size = 16 * 1024 * 1024;
    scexpr size_t _pagesize = 4 * 1024;
    
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


    struct StatusReg:public Reg8<>{
        using Reg8::operator=;

        uint8_t busy:1;
        uint8_t write_enable_latch:1;
        uint8_t block_protect_bits:3;
        uint8_t top_or_bottom_protect:1;
        uint8_t sector_or_block_protect:1;
        uint8_t __resv__:1;
    };

    StatusReg statusReg = {};


    void writePage(const size_t addr, const void * data, const Address len);

    void storeBytes(const Address loc, const void * data, const Address len) override;
    void loadBytes(const Address loc, void * data, const Address len) override;
    void entry_store() override;
    void exit_store() override;
    void entry_load() override;
    void exit_load() override;

public:
    W25Q16(hal::SpiDrv & _spi_drv):StoragePaged(_m_size, _pagesize), spi_drv(_spi_drv){;}
    W25Q16(hal::Spi & _spi, const uint8_t index = 0):StoragePaged(_m_size, _pagesize), spi_drv(hal::SpiDrv(_spi, index)){;}

    void enableWrite(const bool en = true);
    uint8_t getDeviceManufacturer();
    uint8_t getDeviceStorageType();
    uint8_t getDeviceCapacity();

    void eraseBlock(const Address addr);
    void eraseSector(const Address addr);
    void eraseChip();

    bool isIdle();
    bool isWriteable();

private:
    BusError writeByte(const uint8_t data){
        return spi_drv.writeSingle(data);
    }

    BusError writeByte(const Commands & data){
        return spi_drv.writeSingle((uint8_t)data);
    }

    BusError readByte(uint8_t & data){
        return spi_drv.readSingle(data);
    }

};

}