#pragma once

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{
class PAJ7620{
protected:
    I2cDrv i2c_drv_;

    void GESTURE_Actions(void); /* Sets action flags for gestures */

    void writeReg(uint8_t addr, uint8_t cmd); /* Write to addressed register */
    void readReg(uint8_t addr, uint8_t qty, uint8_t * data); /* Read data from addressed register */
    void selectBank(uint8_t bank);
public:
    scexpr uint8_t default_i2c_addr = (0x73) << 1;

    bool verify();
    void init();
    void update();
    uint8_t detect();

    PAJ7620(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    PAJ7620(I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    PAJ7620(I2c & i2c, const uint8_t addr = default_i2c_addr):i2c_drv_(I2cDrv{i2c, addr}){;}
};

}
