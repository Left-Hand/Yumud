#pragma once

#include "IMU.hpp"


namespace ymd::details{
    enum class BoschSensorError:uint8_t{
        UNSPECIFIED = 255,
    };
}

namespace ymd::custom{
    template<typename T>
    struct result_converter<T, details::BoschSensorError, BusError> {
        static Result<T, details::BoschSensorError> convert(const BusError berr){
            using Error = details::BoschSensorError;
            using BusError = BusError;
            
            if(berr.ok()) return Ok();

            Error err = [](const BusError berr_){
                switch(berr_.type){
                    // case BusError::NO_ACK : return Error::I2C_NOT_ACK;

                    // case BusError::I2C_NOT_READY: return _BMI088_Base::Error::I2C_NOT_READY;
                    default: return Error::UNSPECIFIED;
                }
            }(berr);

            return Err(err); 
        }
    };
}


namespace ymd::drivers{

class BoschSensor{
public:
    using Error = details::BoschSensorError;
protected:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;

    [[nodiscard]] __fast_inline
    Result<void, Error> writeReg(const uint8_t addr, const uint8_t data){
        if(i2c_drv_){
            return Result<void, Error>(i2c_drv_->writeReg(addr, data));
        }else if(spi_drv_){
            spi_drv_->writeSingle(uint8_t(addr), CONT).unwrap();
            return Result<void, Error>(spi_drv_->writeSingle(data));
        }

        PANIC();
    }

    [[nodiscard]] __fast_inline
    Result<void, Error> readReg(const uint8_t addr, uint8_t & data){
        if(i2c_drv_){
            return Result<void, Error>(i2c_drv_->readReg(uint8_t(addr), data));
        }else if(spi_drv_){
            spi_drv_->writeSingle(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return Result<void, Error>(spi_drv_->readSingle(data));
        }

        PANIC();
    }

    [[nodiscard]] __fast_inline
    Result<void, Error> readBurst(const uint8_t addr, int16_t * datas, const size_t len){
        if(i2c_drv_){
            return Result<void, Error>(i2c_drv_->readBurst<int16_t>(uint8_t(addr), std::span(datas, len), LSB));
        }else if(spi_drv_){
            spi_drv_->writeSingle<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return Result<void, Error>(spi_drv_->readBurst<uint8_t>(reinterpret_cast<uint8_t *>(datas), len * sizeof(int16_t)));
        }

        PANIC();
    }

    
    [[nodiscard]] __fast_inline
    Result<void, Error> writeCommand(const uint8_t cmd){
        return writeReg(0x7e, cmd);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    Result<void, Error> writeRegs(Ts const & ... reg) {
        return (writeReg(reg.address, reg.as_val()) | ...);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    Result<void, Error> readRegs(Ts & ... reg) {
        return (readReg(reg.address, reg.as_ref()) | ...);
    }
public:

    BoschSensor(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv), spi_drv_(std::nullopt){;}
    BoschSensor(hal::I2c & i2c, const uint8_t addr):
        BoschSensor(hal::I2cDrv{i2c, addr}){;}
    BoschSensor(const hal::SpiDrv & spi_drv):
        i2c_drv_(std::nullopt), spi_drv_(spi_drv){;}
    BoschSensor(hal::Spi & spi, const uint8_t index):
        BoschSensor(hal::SpiDrv{spi, index}){;}
};
}

