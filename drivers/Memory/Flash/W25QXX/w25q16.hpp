#pragma once

#include "drivers/device_defs.h"
#include "drivers/Memory/memory.hpp"

// #define W25Q16_DEBUG

#ifdef W25Q16_DEBUG
#define W25Q16_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define W25Q16_DEBUG(...)
#endif

class W25Q16:public StoragePaged{
protected:
    SpiDrv bus_drv;

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


    struct StatusReg:public Reg8{
        using Reg8::operator=;

        uint8_t busy:1;
        uint8_t write_enable_latch:1;
        uint8_t block_protect_bits:3;
        uint8_t top_or_bottom_protect:1;
        uint8_t sector_or_block_protect:1;
        uint8_t __resv__:1;
    };

    StatusReg statusReg;


    void writePage(const void * data, const Address len, const size_t addr);

    void storeBytes(const uint8_t data, const Address loc) override;
    void loadBytes(uint8_t & data, const Address loc) override;
    void storeBytes(const void * data, const Address data_size, const Address loc) override;
    void loadBytes(void * data, const Address data_size, const Address loc) override;
    void entry_store() override;
    void exit_store() override;
    void entry_load() override;
    void exit_load() override;

public:
    W25Q16(SpiDrv & _bus_drv):StoragePaged(_m_size, _pagesize), bus_drv(_bus_drv){;}
    W25Q16(Spi & _spi, const uint8_t index = 0):StoragePaged(_m_size, _pagesize), bus_drv(SpiDrv(_spi, index)){;}

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
    void writeByte(const uint8_t data){
        bus_drv.write(data);
    }

    void writeByte(const Commands & data){
        bus_drv.write((uint8_t)data);
    }

    void readByte(uint8_t & data){
        bus_drv.read(data);
    }

};