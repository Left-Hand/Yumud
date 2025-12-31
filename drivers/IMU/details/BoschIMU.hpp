#pragma once

#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{

class BoschImu_Transport final{

public:
    using Error = ImuError;
    explicit BoschImu_Transport(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv), spi_drv_(std::nullopt){;}
    explicit BoschImu_Transport(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr):
        BoschImu_Transport(hal::I2cDrv{i2c, addr}){;}
    explicit BoschImu_Transport(const hal::SpiDrv & spi_drv):
        i2c_drv_(std::nullopt), spi_drv_(spi_drv){;}
    explicit BoschImu_Transport(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):
        BoschImu_Transport(hal::SpiDrv{spi, index}){;}

    [[nodiscard]] 
    Result<void, Error> write_reg(const uint8_t addr, const uint8_t data);

    template<typename T>
    [[nodiscard]] __fast_inline
    Result<void, Error> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.to_bits());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }


    template<typename T>
    [[nodiscard]] __fast_inline
    Result<void, Error> read_reg(T & reg){
        if(const auto res = read_reg(T::ADDRESS, reg.as_bits_mut());
            res.is_err()) return res;
        return Ok();
    }

    [[nodiscard]] 
    Result<void, Error> read_reg(const uint8_t addr, uint8_t & data);

    [[nodiscard]] 
    Result<void, Error> read_burst(const uint8_t addr, std::span<int16_t> pbuf);

    
    [[nodiscard]] __fast_inline
    Result<void, Error> write_command(const uint8_t cmd){
        return write_reg(0x7e, cmd);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    Result<void, Error> write_regs(Ts const & ... reg) {
        return (write_reg(reg.ADDRESS, reg.to_bits()) | ...);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    Result<void, Error> read_regs(Ts & ... reg) {
        return (read_reg(reg.ADDRESS, reg.as_bits_mut()) | ...);
    }

    [[nodiscard]] __fast_inline
    Result<void, Error> validate() {
        return Ok();
    }
private:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;
};
}

