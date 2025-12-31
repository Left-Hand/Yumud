#pragma once

#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

#include "core/utils/result.hpp"

namespace ymd::drivers{

class InvensenseImu_Transport final{
public:
    using Error = ImuError;
    [[nodiscard]] Result<void, Error> write_reg(const uint8_t addr, const uint8_t data);

    [[nodiscard]] Result<void, Error> read_reg(const uint8_t addr, uint8_t & data);

    [[nodiscard]] Result<void, Error> read_burst(const uint8_t addr, std::span<int16_t> pbuf);
    [[nodiscard]] Result<void, Error> write_command(const uint8_t command){
        if(const auto res = this->write_reg(0x7e, command);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
public:
    explicit InvensenseImu_Transport(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit InvensenseImu_Transport(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit InvensenseImu_Transport(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}
    explicit InvensenseImu_Transport(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit InvensenseImu_Transport(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit InvensenseImu_Transport(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):
        spi_drv_(hal::SpiDrv{spi, index}){;}

    [[nodiscard]] Result<void, Error> reset(){
        if(i2c_drv_){
            if(const auto res = i2c_drv_->release();
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }
private:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;
};
}