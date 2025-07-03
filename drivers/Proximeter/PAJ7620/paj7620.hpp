#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{
class PAJ7620 final{
public:
    enum class Error_Kind:uint8_t{
        Data0ValidateFailed,
        Data1ValidateFailed
    };

    FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7((0x73));

    PAJ7620(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}

    PAJ7620(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    PAJ7620(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}


    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();

    struct Flags{
        uint8_t right:1;
        uint8_t left:1;
        uint8_t up:1;
        uint8_t down:1;
        uint8_t forward:1;
        uint8_t backward:1;
        uint8_t cw:1;
        uint8_t ccw:1;

        constexpr std::bitset<8> to_bitset() const{
            return std::bitset<8>(std::bit_cast<uint8_t>(*this));
        }
    };

    static_assert(sizeof(Flags) == 1);

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
