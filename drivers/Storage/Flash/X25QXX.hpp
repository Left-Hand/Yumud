#pragma once


#include "core/io/regs.hpp"
#include "core/clock/clock.hpp"

#include "concept/storage.hpp"
#include "concept/jedec.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"


#if 0
namespace ymd::drivers{

struct X25QXX_Prelude:public StorageCollections{

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
};

class X25QXX_Phy final:public StorageCollections{
    hal::HalResult write_byte(const uint8_t data, const Continuous cont = DISC){
        return spi_drv_.write_single<uint8_t>(data, cont);
    }

    hal::HalResult write_byte(const Command cmd, const Continuous cont = DISC){
        return write_byte(uint8_t(cmd), cont);
    }

    hal::HalResult write_bytes(const void * data, const size_t len){
        return spi_drv_.write_burst<uint8_t>(reinterpret_cast<const uint8_t *>(data), len);
    }

    hal::HalResult read_byte(uint8_t & data, const Continuous cont = DISC){
        return spi_drv_.read_single<uint8_t>(data, cont);
    }

    hal::HalResult read_bytes(void * data, const size_t len){
        // DEBUGGER.print("nr");
        // DEBUGGER.print_arr(reinterpret_cast<uint8_t *>(data), len);
        // DEBUGGER.println("nr!");
        return spi_drv_.read_burst<uint8_t>(reinterpret_cast<uint8_t *>(data), len);
        // DEBUGGER.print("ar");
        // DEBUGGER.print_arr(reinterpret_cast<uint8_t *>(data), len);
        // DEBUGGER.println("ar!");
    }
};

class X25QXX_Regs:public X25QXX_Prelude{

};

class X25QXX final: public StorageIntf{

public:
    X25QXX(const hal::SpiDrv & spi_drv, const Address capacity):
        spi_drv_(spi_drv)
        
        {;}

    X25QXX(hal::SpiDrv && spi_drv, const Address capacity):
        StoragePaged(capacity, 256),
        spi_drv_(std::move(spi_drv))
    
        {;}
    void init(){}

    bool is_busy();


    void enable_write(const Enable en){
        if(en){
            write_byte(Command::WriteEnable);
        }else{
            write_byte(Command::WriteDisable);
        }
    }

    JedecManufacturer get_device_manufacturer(){
        update_jedec_id();
        return JedecManufacturer(jedec_id.manufacturer_id);
    }

    JedecStorageType get_device_storage_type(){
        update_jedec_id();
        return JedecStorageType(jedec_id.memory_type);
    }

    size_t get_device_capacity(){
        update_jedec_id();
        return 1 << jedec_id.capacity;
    }

    void enable_power_down(const Enable en);


    bool is_writeable();

    void erase_bytes(const Address loc, const size_t len);
private:
protected:
    hal::SpiDrv spi_drv_;


    StatusReg statusReg;
    JedecId jedec_id;




    void write_addr(const Address addr, const Continuous cont = DISC);
    void skip_byte(){write_byte(0, CONT);}

    void load_bytes(const Address loc, std::span<const >);
    void store_bytes(const Address loc, const void * data, const Address len);

    void update_device_id();

    void update_jedec_id();

    void update_status();

    void write_page(const Address addr, const uint8_t * data, size_t len);

    bool wait_for_free(Milliseconds timeout);

    bool is_large_chip(){return capacity_ > 0x1000000;}


    void erase_sector(const Address addr);

    void erase_block(const Address addr);

    void erase_whole_chip();

};

}
#endif