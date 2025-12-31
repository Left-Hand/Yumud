#pragma once

#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

class AsahiKaseiImu_Transport final{

public:
    AsahiKaseiImu_Transport(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    AsahiKaseiImu_Transport(hal::I2cDrv && i2c_drv):
        i2c_drv_(i2c_drv){;}
    AsahiKaseiImu_Transport(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}
    AsahiKaseiImu_Transport(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    AsahiKaseiImu_Transport(hal::SpiDrv && spi_drv):
        spi_drv_(spi_drv){;}
    AsahiKaseiImu_Transport(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv{spi, rank}){;}

    [[nodiscard]] Result<void, ImuError> write_reg(const uint8_t addr, const uint8_t data);

    [[nodiscard]] Result<void, ImuError> read_reg(const uint8_t addr, uint8_t & data);

    [[nodiscard]] Result<void, ImuError> read_burst(const uint8_t addr, std::span<int16_t> pbuf);

    [[nodiscard]] Result<void, ImuError> read_burst(const uint8_t addr, const std::span<uint8_t> pbuf);

    [[nodiscard]] Result<void, ImuError> validate();
private:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;
};
}