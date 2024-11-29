#pragma once

#include "drivers/device_defs.h"


namespace ymd::drivers{
class PAJ7620{
protected:
    I2cDrv i2c_drv_;

    void writeReg(uint8_t addr, uint8_t cmd); /* Write to addressed register */
    void readReg(uint8_t addr, uint8_t & data); /* Read data from addressed register */
    void selectBank(uint8_t bank);

    struct Flags{
        uint8_t right:1;
        uint8_t left:1;
        uint8_t up:1;
        uint8_t down:1;
        uint8_t forward:1;
        uint8_t backward:1;
        uint8_t cw:1;
        uint8_t ccw:1;

        operator uint8_t &(){return *reinterpret_cast<uint8_t *>(this);}
        operator const uint8_t &()const {return *reinterpret_cast<const uint8_t *>(this);}
    };

    Flags flags;

    void unlock_i2c();//bad i2c device;
    
    // void wakeup();
public:
    scexpr uint8_t default_i2c_addr = (0x73) << 1;

    bool verify();
    void init();
    void update();
    Flags detect();
    PAJ7620(I2c & i2c, const uint8_t addr = default_i2c_addr):i2c_drv_(I2cDrv{i2c, addr}){;}

    PAJ7620(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    PAJ7620(I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
};

}
