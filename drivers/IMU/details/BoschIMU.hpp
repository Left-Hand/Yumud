#pragma once

#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{

class BoschSensor_Phy final{

public:
    using Error = ImuError;
    BoschSensor_Phy(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv), spi_drv_(std::nullopt){;}
    BoschSensor_Phy(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr):
        BoschSensor_Phy(hal::I2cDrv{i2c, addr}){;}
    BoschSensor_Phy(const hal::SpiDrv & spi_drv):
        i2c_drv_(std::nullopt), spi_drv_(spi_drv){;}
    BoschSensor_Phy(Some<hal::Spi *> spi, const hal::SpiSlaveIndex index):
        BoschSensor_Phy(hal::SpiDrv{spi, index}){;}

    [[nodiscard]] __fast_inline
    Result<void, Error> write_reg(const uint8_t addr, const uint8_t data){
        if(i2c_drv_){
            return Result<void, Error>(i2c_drv_->write_reg(addr, data));
        }else if(spi_drv_){
            return Result<void, Error>(
                spi_drv_->write_single<uint8_t>(uint8_t(addr), CONT) |
                spi_drv_->write_single<uint8_t>(data)
            );
        }

        return Err(Error::NoAvailablePhy);
    }

    template<typename T>
    [[nodiscard]] __fast_inline
    Result<void, Error> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(reg.address, reg.as_val());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }

    [[nodiscard]] __fast_inline
    Result<void, Error> read_reg(const uint8_t addr, uint8_t & data){
        if(i2c_drv_){
            return Result<void, Error>(i2c_drv_->read_reg(uint8_t(addr), data));
        }else if(spi_drv_){
            return Result<void, Error>(
                spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT) |
                spi_drv_->read_single<uint8_t>(data)
            );
        }


        return Err(Error::NoAvailablePhy);
    }

    [[nodiscard]] __fast_inline
    Result<void, Error> read_burst(const uint8_t addr, int16_t * datas, const size_t len){
        if(i2c_drv_){
            return Result<void, Error>(i2c_drv_->read_burst<int16_t>(uint8_t(addr), std::span(datas, len), LSB));
        }else if(spi_drv_){
            return Result<void, Error>(
                spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT)
                | spi_drv_->read_burst<uint8_t>(std::span(reinterpret_cast<uint8_t *>(datas), len * sizeof(int16_t)))
            );
        }

        return Err(Error::NoAvailablePhy);
    }

    
    [[nodiscard]] __fast_inline
    Result<void, Error> write_command(const uint8_t cmd){
        return write_reg(0x7e, cmd);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    Result<void, Error> write_regs(Ts const & ... reg) {
        return (write_reg(reg.address, reg.as_val()) | ...);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    Result<void, Error> read_regs(Ts & ... reg) {
        return (read_reg(reg.address, reg.as_ref()) | ...);
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

