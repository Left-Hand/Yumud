#pragma once

#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

#include "core/utils/result.hpp"

namespace ymd::drivers{



class InvensenseSensorError{
public:
    enum Kind:uint8_t{
        BusError,
        WrongWhoAmI,
        Unspecified = 0xff,
    };

    constexpr InvensenseSensorError(Kind kind):kind_(kind){;} 
    constexpr bool operator==(const InvensenseSensorError & rhs) const{return kind_ == rhs.kind_;}
private:
    Kind kind_;
};


class InvensenseSensor_Phy final{
public:
    BusError write_reg(const uint8_t addr, const uint8_t data) {
        if(i2c_drv_){
            return i2c_drv_->write_reg(uint8_t(addr), data);
        }else if(spi_drv_){
            spi_drv_->write_single(uint8_t(addr), CONT).unwrap();
            return spi_drv_->write_single(data);
        }

        PANIC();
    }

    BusError read_reg(const uint8_t addr, uint8_t & data) {
        if(i2c_drv_){
            return i2c_drv_->read_reg(uint8_t(addr), data);
        }else if(spi_drv_){
            spi_drv_->write_single(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return spi_drv_->read_single(data);
        }

        PANIC();
    }

    BusError read_burst(const uint8_t addr, int16_t * datas, const size_t len){
        if(i2c_drv_){
            return i2c_drv_->read_burst<int16_t>(uint8_t(addr), std::span(datas, len), LSB);
        }else if(spi_drv_){
            spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT).unwrap();
            return spi_drv_->read_burst<uint8_t>(reinterpret_cast<uint8_t *>(datas), len * sizeof(int16_t));
        }

        PANIC();
    }

    BusError write_command(const uint8_t command){
        return this->write_reg(0x7e, command);
    }
public:
    InvensenseSensor_Phy(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    InvensenseSensor_Phy(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    InvensenseSensor_Phy(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}
    InvensenseSensor_Phy(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    InvensenseSensor_Phy(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    InvensenseSensor_Phy(hal::Spi & spi, const hal::SpiSlaveIndex index):spi_drv_(hal::SpiDrv{spi, index}){;}

private:
    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;
};
}


namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::InvensenseSensorError, BusError> {
        static Result<T, drivers::InvensenseSensorError> convert(const BusError berr){
            using Error = drivers::InvensenseSensorError;
            using BusError = BusError;
            
            if(berr.ok()) return Ok();

            Error err = [](const BusError berr_){
                switch(berr_.type){
                    // case BusError::NO_ACK : return Error::I2C_NOT_ACK;

                    // case BusError::I2C_NOT_READY: return _BMI088_Base::Error::I2C_NOT_READY;
                    default: return Error::Unspecified;
                }
            }(berr);

            return Err(err); 
        }
    };
}
