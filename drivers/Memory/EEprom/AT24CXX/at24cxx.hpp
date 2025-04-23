#pragma once

#include "core/io/regs.hpp"
#include "concept/memory.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

class AT24CXX:public StoragePaged{
private:
    constexpr bool is_small_chip(){return capacity_ <= 256;}
protected:
    scexpr uint32_t min_duration_ms = 6;

    hal::I2cDrv i2c_drv_;
    uint32_t last_entry_ms = 0;

    void store_bytes(const Address loc, const void * data, const Address len) override;

    void load_bytes(const Address loc, void * data, const Address len) override;

    void write_pool(const size_t addr, const uint8_t * data, const size_t len);

    void read_pool(const size_t addr, uint8_t * data, const size_t len);

    void entry_store() override{
        update_entry_ms();
    };

    void exit_store() override{
        // last_entry_ms = millis();
        update_entry_ms();
    };

    void entry_load() override{
        // last_entry_ms = millis();
        wait_for_free();
        update_entry_ms();
    };

    void exit_load() override{
        // last_entry_ms = millis();
        update_entry_ms();
    };

    AT24CXX(hal::I2c & _bus, const Address _m_size, const Address _pagesize):StoragePaged(_m_size, _pagesize), i2c_drv_{_bus, DEFAULT_I2C_ADDR}{;}
    AT24CXX(hal::I2cDrv && i2c_drv, const Address _m_size, const Address _pagesize):StoragePaged(_m_size, _pagesize), i2c_drv_(i2c_drv){;}
    AT24CXX(hal::I2cDrv & i2c_drv, const Address _m_size, const Address _pagesize):StoragePaged(_m_size, _pagesize), i2c_drv_(i2c_drv){;}

public:
    void init() override{};

    bool busy() override{return last_entry_ms + min_duration_ms - millis() > 0;}

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b10100000); 
private:
    void wait_for_free();

    void update_entry_ms(){last_entry_ms = millis();}
};

#define AT24CXX_DEF_TEMPLATE(name, size, pagesize)\
class AT24C##name:public AT24CXX{\
public:\
    AT24C##name(hal::I2c & _bus):AT24CXX(_bus, size, pagesize){;}\
    AT24C##name(hal::I2cDrv & i2c_drv):AT24CXX(i2c_drv, size, pagesize){;}\
    AT24C##name(hal::I2cDrv && i2c_drv):AT24CXX(i2c_drv, size, pagesize){;}\
};\

AT24CXX_DEF_TEMPLATE(01, 1 << 7, 8)
AT24CXX_DEF_TEMPLATE(02, 1 << 8, 8)
AT24CXX_DEF_TEMPLATE(04, 1 << 9, 16)
AT24CXX_DEF_TEMPLATE(08, 1 << 10, 16)
AT24CXX_DEF_TEMPLATE(16, 1 << 11, 16)
AT24CXX_DEF_TEMPLATE(32, 1 << 12, 32)
AT24CXX_DEF_TEMPLATE(64, 1 << 13, 32)
AT24CXX_DEF_TEMPLATE(128, 1 << 14, 64)
AT24CXX_DEF_TEMPLATE(256, 1 << 15, 64)
AT24CXX_DEF_TEMPLATE(512, 1 << 16, 128)

#undef AT24CXX_DEF_TEMPLATE

}