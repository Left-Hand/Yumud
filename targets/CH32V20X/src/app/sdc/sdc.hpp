#pragma once


#include "drivers/device_defs.h"
#include "concept/storage.hpp"
#include "jedec.hpp"

#define W25QXX_DEBUG

#ifdef W25QXX_DEBUG
#undef W25QXX_DEBUG
#define W25QXX_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define W25QXX_DEBUG(...)
#endif




class SpiDevice{
protected:
    SpiDrv spi_drv_;

    SpiDevice(const SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    SpiDevice(SpiDrv && spi_drv):spi_drv_(spi_drv){;}
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

    struct StatusReg:public Reg8{
        using Reg8::operator=;
        uint8_t busy:1;
        uint8_t write_enable_latch:1;
        uint8_t block_protect_bits:3;
        uint8_t top_or_bottom_protect:1;
        uint8_t sector_or_block_protect:1;
        uint8_t __resv__:1;
    };

    struct JedecId:public Reg24{
        using Reg24::operator=;
        uint8_t capacity;
        uint8_t memory_type;
        uint8_t manufacturer_id;
    };

    StatusReg statusReg;
    JedecId jedec_id;


    void writeByte(const uint8_t data, const Continuous cont = DISC){
        spi_drv_.writeSingle<uint8_t>(data, cont);
    }

    void writeByte(const Command data, const Continuous cont = DISC){
        spi_drv_.writeSingle<uint8_t>(uint8_t(data), cont);
    }

    void readByte(uint8_t & data, const Continuous cont = DISC){
        spi_drv_.readSingle<uint8_t>(data, cont);
    }

    void writeAddr(const Address addr, const Continuous cont = DISC){
        writeByte(addr >> 16, CONT);
        writeByte(addr >> 8, CONT);
        writeByte(addr, cont);
    }

    void skipByte(){
        writeByte(0, CONT);
    }

    void wait_for_free(){

    }

    void entry_store() override{}
    void exit_store() override{}

    void entry_load() override{}
    void exit_load() override{}

    void loadBytes(void * data, const Address len, const Address loc) override{}
    void storeBytes(const void * data, const Address len, const Address loc) override{};

    void updateDeviceId(){
        writeByte(0x90, CONT);
        skipByte();
        skipByte();
        skipByte();

        readByte(jedec_id[2], CONT);
        readByte(jedec_id[0]);
    }

    void updateJedecId(){
        writeByte(0x9F, CONT);

        readByte(jedec_id[2], CONT);
        readByte(jedec_id[1], CONT);
        readByte(jedec_id[0]);
    }

public:

    void init() override{}

    bool busy() override{return false;}


public:
    W25QXX(const SpiDrv & spi_drv):SpiDevice(spi_drv), StoragePaged(32_MB, 256){;}
    W25QXX(SpiDrv && spi_drv):SpiDevice(spi_drv), StoragePaged(32_MB, 256){;}

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

    void enablePowerDown(const bool en = true   ){
        writeByte(en ? Command::PowerDown : Command::ReleasePowerDown);
    }

    void eraseBlock(const Address addr){
        enableWrite();
        writeByte(Command::SectorErase, CONT);
        writeAddr(addr);
    }

    void eraseSector(const Address addr){
        enableWrite();
        writeByte(Command::SectorErase, CONT);
        writeAddr(addr);
    }
    void eraseChip(){
        writeByte(Command::ChipErase);
    }

    bool isIdle(){
        writeByte(Command::ReadStatusRegister, CONT);
        readByte(statusReg);
        return statusReg.busy;
    }

    bool isWriteable(){
        writeByte(Command::ReadStatusRegister);
        readByte(statusReg);
        return statusReg.write_enable_latch;
    }

    void writePage(const Address addr, const uint8_t * data, size_t len){
        enableWrite();
        if(len > 256){
            W25QXX_DEBUG("page too large", len);
            len = 256;
        }
        writeByte(Command::PageProgram, CONT);
        writeAddr(addr, CONT);
        for(size_t i = 0; i < len; i++){
            writeByte(data[i], CONT);
        }
    }

    void writeData(const Address _addr, const uint8_t * _data, const size_t len){
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

    void readData(const Address addr, uint8_t * data, const size_t len){
        writeByte(Command::ReadData);
        writeByte(addr >> 16);
        writeByte(addr >> 8);
        writeByte(addr);
        for(size_t i = 0; i < len; i++){
            readByte(data[i]);
        }
    }
};