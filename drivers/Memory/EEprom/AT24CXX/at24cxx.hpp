#pragma once

#include "drivers/device_defs.h"
#include "../../memory.hpp"

class AT24CXX:public Storage{
protected:
    I2cDrv bus_drv;
    static constexpr Address page_size = 8;
    static constexpr uint32_t min_duration_ms = 7;
    uint32_t last_entry_ms = 0;
    
    void _store(const uint8_t data, const Address loc) override;
    void _load(uint8_t & data, const Address loc) override;

    void wait_for_done();

    void update_entry_ms(){last_entry_ms = millis();}

    void _store(const void * data, const Address data_size, const Address loc) override;

    void _load(void * data, const Address data_size, const Address loc) override;

    void entry_store() override{
        update_entry_ms();
    };

    void exit_store() override{
        // last_entry_ms = millis();
        update_entry_ms();
    };

    void entry_load() override{
        // last_entry_ms = millis();
        wait_for_done();
        update_entry_ms();
    };

    void exit_load() override{
        // last_entry_ms = millis();
        update_entry_ms();
    };
public:
    void init() override{};

    bool busy() override{return last_entry_ms + min_duration_ms - millis() > 0;}

    static constexpr uint8_t default_id = 0b10100000; 

    AT24CXX(I2cDrv && _bus_drv, const Address _m_size):Storage(_m_size), bus_drv(_bus_drv){;}
    AT24CXX(I2c & _bus, const Address _m_size):Storage(_m_size), bus_drv{_bus, default_id}{;}
    AT24CXX(I2cDrv & _bus_drv, const Address _m_size):Storage(_m_size), bus_drv(_bus_drv){;}
};

#define AT24CXX_DEF_TEMPLATE(name, size)\
class AT24C##name:public AT24CXX{\
public:\
    AT24C##name(I2c & _bus):AT24CXX(_bus, size){;}\
    AT24C##name(I2cDrv & _bus_drv):AT24CXX(_bus_drv, size){;}\
    AT24C##name(I2cDrv && _bus_drv):AT24CXX(_bus_drv, size){;}\
};\

AT24CXX_DEF_TEMPLATE(01, 1 << 7)
AT24CXX_DEF_TEMPLATE(02, 1 << 8)
AT24CXX_DEF_TEMPLATE(04, 1 << 9)
AT24CXX_DEF_TEMPLATE(08, 1 << 10)
AT24CXX_DEF_TEMPLATE(16, 1 << 11)
AT24CXX_DEF_TEMPLATE(32, 1 << 12)
AT24CXX_DEF_TEMPLATE(64, 1 << 13)

#undef AT24CXX_DEF_TEMPLATE

