#pragma once

#include "core/io/regs.hpp"
#include "concept/memory.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd{


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


    void write_page(const size_t addr, const void * data, const Address len);

    void store_bytes(const Address loc, const void * data, const Address len) override;
    void load_bytes(const Address loc, void * data, const Address len) override;
    void entry_store() override;
    void exit_store() override;
    void entry_load() override;
    void exit_load() override;

public:
    W25Q16(hal::SpiDrv & _spi_drv):StoragePaged(_m_size, _pagesize), spi_drv(_spi_drv){;}
    W25Q16(hal::Spi & _spi, const hal::SpiSlaveIndex index):StoragePaged(_m_size, _pagesize), spi_drv(hal::SpiDrv(_spi, index)){;}

    void enable_write(const bool en = true);
    uint8_t get_device_manufacturer();
    uint8_t get_device_storage_type();
    uint8_t get_device_capacity();

    void erase_block(const Address addr);
    void erase_sector(const Address addr);
    void erase_chip();

    bool is_idle();
    bool is_writeable();

private:
    hal::BusError write_byte(const uint8_t data){
        return spi_drv.write_single<uint8_t>(data);
    }

    hal::BusError write_byte(const Commands & data){
        return spi_drv.write_single<uint8_t>((uint8_t)data);
    }

    hal::BusError read_byte(uint8_t & data){
        return spi_drv.read_single<uint8_t>(data);
    }

};

}