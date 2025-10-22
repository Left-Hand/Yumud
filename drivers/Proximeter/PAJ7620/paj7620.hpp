#pragma once

#include "paj7620_prelude.hpp"

namespace ymd::drivers{


class PAJ7620 final:
    public PAJ7620_Prelude{
public:

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7((0x73));

    explicit PAJ7620(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}

    explicit PAJ7620(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    explicit PAJ7620(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}


    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();


    [[nodiscard]] IResult<Flags> detect();

private:
    hal::I2cDrv i2c_drv_;

    Flags flags;

    enum class Bank:uint8_t{
        _0, _1
    };

    //  Write to addressed register
    [[nodiscard]] IResult<> write_reg(uint8_t addr, uint8_t cmd);
    // Read data from addressed register
    [[nodiscard]] IResult<> read_reg(uint8_t addr, uint8_t & data);
    [[nodiscard]] IResult<> select_bank(Bank bank);
    [[nodiscard]] IResult<> unlock_i2c();//bad i2c device;
    
};

}
