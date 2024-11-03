#pragma once

#include "drivers/device_defs.h"
#include "drivers/Memory/memory.hpp"

namespace yumud{

class AT24CXX:public StoragePaged{
private:
    constexpr bool is_small_chip(){return m_capacity <= 256;}
protected:
    scexpr uint32_t min_duration_ms = 6;

    I2cDrv bus_drv;
    uint32_t last_entry_ms = 0;
    

    void storeBytes(const uint8_t data, const Address loc) override;
    void loadBytes(uint8_t & data, const Address loc) override;

    void storeBytes(const void * data, const Address data_size, const Address loc) override;

    void loadBytes(void * data, const Address data_size, const Address loc) override;

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

    AT24CXX(I2cDrv && _bus_drv, const Address _m_size, const Address _pagesize):StoragePaged(_m_size, _pagesize), bus_drv(_bus_drv){;}
    AT24CXX(I2c & _bus, const Address _m_size, const Address _pagesize):StoragePaged(_m_size, _pagesize), bus_drv{_bus, default_id}{;}
    AT24CXX(I2cDrv & _bus_drv, const Address _m_size, const Address _pagesize):StoragePaged(_m_size, _pagesize), bus_drv(_bus_drv){;}

public:
    void init() override{};

    bool busy() override{return last_entry_ms + min_duration_ms - millis() > 0;}

    scexpr uint8_t default_id = 0b10100000; 
private:
    void wait_for_free();

    void update_entry_ms(){last_entry_ms = millis();}
};

#define AT24CXX_DEF_TEMPLATE(name, size, pagesize)\
class AT24C##name:public AT24CXX{\
public:\
    AT24C##name(I2c & _bus):AT24CXX(_bus, size, pagesize){;}\
    AT24C##name(I2cDrv & _bus_drv):AT24CXX(_bus_drv, size, pagesize){;}\
    AT24C##name(I2cDrv && _bus_drv):AT24CXX(_bus_drv, size, pagesize){;}\
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