#pragma once


#include "drivers/device_defs.h"
#include "concept/storage.hpp"
#include "concept/jedec.hpp"

#define X25QXX_DEBUG

#ifdef X25QXX_DEBUG
#undef X25QXX_DEBUG
#define X25QXX_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define X25QXX_DEBUG(...)
#endif

namespace yumud::drivers{


class SpiDevice{
protected:
    SpiDrv spi_drv_;

    SpiDevice(const SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    SpiDevice(SpiDrv && spi_drv):spi_drv_(spi_drv){;}
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

    void writeByte(const Command cmd, const Continuous cont = DISC){
        writeByte(uint8_t(cmd), cont);
    }

    void writeBytes(const void * data, const size_t len){
        spi_drv_.writeMulti<uint8_t>(reinterpret_cast<const uint8_t *>(data), len);
    }

    void readByte(uint8_t & data, const Continuous cont = DISC){
        spi_drv_.readSingle<uint8_t>(data, cont);
    }

    void readBytes(void * data, const size_t len){
        DEBUGGER.print("nr");
        DEBUGGER.print_arr(reinterpret_cast<uint8_t *>(data), len);
        DEBUGGER.println("nr!");
        spi_drv_.readMulti<uint8_t>(reinterpret_cast<uint8_t *>(data), len);
        DEBUGGER.print("ar");
        DEBUGGER.print_arr(reinterpret_cast<uint8_t *>(data), len);
        DEBUGGER.println("ar!");
    }

    void writeAddr(const Address addr, const Continuous cont = DISC){
        writeByte(addr >> 16, CONT);
        writeByte(addr >> 8, CONT);
        writeByte(addr, cont);
    }

    void skipByte(){
        writeByte(0, CONT);
    }

    void entry_store() override;
    void exit_store() override;

    void entry_load() override;
    void exit_load() override;

    void loadBytes(void * data, const Address len, const Address loc) override;
    void storeBytes(const void * data, const Address len, const Address loc) override;

    void updateDeviceId();

    void updateJedecId();

    void writePage(const Address addr, const uint8_t * data, size_t len);
public:
    X25QXX(const SpiDrv & spi_drv):SpiDevice(spi_drv), StoragePaged(32_MB, 256){;}
    X25QXX(SpiDrv && spi_drv):SpiDevice(std::move(spi_drv)), StoragePaged(32_MB, 256){;}

    void init() override{}

    bool busy() override{return false;}


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

    void eraseBlock(const Address addr);

    void eraseSector(const Address addr);

    void eraseChip();

    bool isIdle();

    bool isWriteable();



};

}