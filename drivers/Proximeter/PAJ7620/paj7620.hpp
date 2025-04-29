#pragma once

#include "core/io/regs.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{
class PAJ7620 final{
protected:
    hal::I2cDrv i2c_drv_;

    void write_reg(uint8_t addr, uint8_t cmd); /* Write to addressed register */
    void read_reg(uint8_t addr, uint8_t & data); /* Read data from addressed register */
    void select_bank(uint8_t bank);

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

    hal::HalResult unlock_i2c();//bad i2c device;
    
    // void wakeup();
public:
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7((0x73));

    bool verify();
    void init();
    void update();
    Flags detect();
    PAJ7620(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}

    PAJ7620(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    PAJ7620(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
};

}
