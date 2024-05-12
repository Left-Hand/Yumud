#ifndef __AT24C02_HPP__

#define __AT24C02_HPP__

#include "device/device_defs.h"
#include "../../memory.hpp"

#ifdef AT24C02_DEBUG
#define AT24C02_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define AT24C02_DEBUG(...)
#endif

class AT24C02:public Storage{
protected:
    I2cDrv & bus_drv;
    static constexpr Address chip_size = 256;
    static constexpr uint32_t max_page = 8;
    static constexpr uint32_t min_duration_ms = 5;
    uint32_t last_entry_ms = 0;
    
    void _store(const uint8_t & data, const Address & loc) override{
        if(loc >= chip_size){
            AT24C02_DEBUG("AT24C02: writeData: invalid addr or len");
            return;
            // len = MIN(chip_size, addr + len) - addr;
        }

        bus_drv.writeReg((uint8_t)loc, data);
    }
    void _load(uint8_t & data, const Address & loc) override{
        if(loc >= chip_size){
            AT24C02_DEBUG("AT24C02: writeData: invalid addr or len");
            return;
            // len = MIN(chip_size, addr + len) - addr;
        }

        bus_drv.readReg((uint8_t)loc, data);
    }
    // void _load(void * data, const Address & data_size, const Address & loc) = 0;

    void entry() override{
        delay(MAX(last_entry_ms + min_duration_ms - millis(), 0));
        last_entry_ms = millis();
    };

    void exit() override{
        last_entry_ms = millis();
    };
public:
    void init() override{};

    static constexpr uint8_t default_id = 0b10100000; 
    AT24C02(I2cDrv & _bus_drv):Storage(256), bus_drv(_bus_drv){;}
};

#endif
