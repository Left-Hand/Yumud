#pragma once


#include "drivers/device_defs.h"
#include "concept/storage.hpp"
#include "concept/jedec.hpp"



namespace ymd::drivers{


class SpiDevice{
protected:
    hal::SpiDrv spi_drv_;

    SpiDevice(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    SpiDevice(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
};


class X25QXX:public SpiDevice, public StoragePaged{
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

    struct StatusReg:public Reg8<>{
        using Reg8::operator=;
        uint8_t busy:1;
        uint8_t write_enable_latch:1;
        uint8_t block_protect_bits:3;
        uint8_t top_or_bottom_protect:1;
        uint8_t sector_or_block_protect:1;
        uint8_t __resv__:1;
    };

    struct JedecId{
        uint8_t capacity;
        uint8_t memory_type;
        uint8_t manufacturer_id;
    };

    StatusReg statusReg;
    JedecId jedec_id;


    BusError writeByte(const uint8_t data, const Continuous cont = DISC){
        return spi_drv_.writeSingle<uint8_t>(data, cont);
    }

    BusError writeByte(const Command cmd, const Continuous cont = DISC){
        return writeByte(uint8_t(cmd), cont);
    }

    BusError writeBytes(const void * data, const size_t len){
        return spi_drv_.writeBurst<uint8_t>(reinterpret_cast<const uint8_t *>(data), len);
    }

    BusError readByte(uint8_t & data, const Continuous cont = DISC){
        return spi_drv_.readSingle<uint8_t>(data, cont);
    }

    BusError readBytes(void * data, const size_t len){
        // DEBUGGER.print("nr");
        // DEBUGGER.print_arr(reinterpret_cast<uint8_t *>(data), len);
        // DEBUGGER.println("nr!");
        return spi_drv_.readBurst<uint8_t>(reinterpret_cast<uint8_t *>(data), len);
        // DEBUGGER.print("ar");
        // DEBUGGER.print_arr(reinterpret_cast<uint8_t *>(data), len);
        // DEBUGGER.println("ar!");
    }

    void writeAddr(const Address addr, const Continuous cont = DISC);
    void skipByte(){writeByte(0, CONT);}

    void entry_store() override;
    void exit_store() override;

    void entry_load() override;
    void exit_load() override;

    void loadBytes(const Address loc, void * data, const Address len) override;
    void storeBytes(const Address loc, const void * data, const Address len) override;

    void updateDeviceId();

    void updateJedecId();

    void updateStatus();

    void writePage(const Address addr, const uint8_t * data, size_t len);

    bool waitForFree(size_t timeout);

    bool isLargeChip(){return capacity_ > 0x1000000;}


    void eraseSector(const Address addr);

    void eraseBlock(const Address addr);

    void eraseWholeChip();

public:
    X25QXX(const hal::SpiDrv & spi_drv, const Address capacity):SpiDevice(spi_drv), StoragePaged(capacity, 256){;}

    X25QXX(hal::SpiDrv && spi_drv, const Address capacity):SpiDevice(std::move(spi_drv)), StoragePaged(capacity, 256){;}
    void init() override{}

    bool busy() override;


    void enableWrite(const bool en = true){
        if(en){
            writeByte(Command::WriteEnable);
        }else{
            writeByte(Command::WriteDisable);
        }
    }

    JedecManufacturer getDeviceManufacturer(){
        updateJedecId();
        return JedecManufacturer(jedec_id.manufacturer_id);
    }

    JedecStorageType getDeviceStorageType(){
        updateJedecId();
        return JedecStorageType(jedec_id.memory_type);
    }

    size_t getDeviceCapacity(){
        updateJedecId();
        return 1 << jedec_id.capacity;
    }

    void enablePowerDown(const bool en = true);


    bool isWriteable();

    void eraseBytes(const Address loc, const size_t len) override;
};

}