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
    const Address chip_size = 256;
    const uint32_t max_page = 8;

    // void _store(const void * data, const Address & data_size, const Address & loc) override;
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

    void entry() override{};
    void exit() override{};
public:
    void init() override{};

    static constexpr uint8_t default_id = 0b10100000; 
    AT24C02(I2cDrv & _bus_drv):Storage(256), bus_drv(_bus_drv){;}

    // void writeData(const uint8_t & addr, const uint8_t * data, uint8_t len){
    //     if(addr + len > chip_size || len > max_page){
    //         AT24C02_DEBUG("AT24C02: writeData: invalid addr or len");
    //         len = MIN(chip_size, addr + len) - addr;
    //     }
    //     bus_drv.writePool(addr, data, 1, len);
    // }

    // void writeData(const uint8_t & addr, const uint8_t & data){
    //     AT24C02_DEBUG("AT24C02 write: ", addr, data);
    //     writeData(addr, &data, 1);
    // }

    // void readData(const uint8_t & addr, uint8_t * data, uint8_t len){
    //     if(addr + len > chip_size || len > max_page){
    //         AT24C02_DEBUG("AT24C02: readData: invalid addr or len");
    //         len = MIN(chip_size, addr + len) - addr;
    //     }
    //     bus_drv.readPool(addr, data, 1, len);
    // }

    // void readData(const uint8_t & addr, uint8_t * data){
    //     AT24C02_DEBUG("AT24C02 read: ", addr, *data);
    //     readData(addr, data, 1);
    // }
};

#endif
