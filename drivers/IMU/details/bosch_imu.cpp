#include "BoschIMU.hpp"
#include "InvensenseIMU.hpp"
#include "STMicroIMU.hpp"
#include "AsahiKaseiIMU.hpp"
#include "AnalogDeviceIMU.hpp"

namespace ymd::drivers{

[[nodiscard]] Result<void, BoschImu_Transport::Error> 
BoschImu_Transport::write_reg(const uint8_t reg_addr, const uint8_t data){
    // DEBUG_PRINTLN(std::hex, std::showbase, "write_reg", reg_addr, data);
    if(i2c_drv_){
        if(const auto res = i2c_drv_->write_reg(reg_addr, data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        // if(const auto res = spi_drv_->write_single<uint8_t>(reg_addr, CONT);
        //     res.is_err()) return Err(res.unwrap_err());
        // if(const auto res = spi_drv_->write_single<uint8_t>(data);
        //     res.is_err()) return Err(res.unwrap_err());
        const std::array<uint8_t, 2> bytes = {reg_addr, data};
        if(const auto res = spi_drv_->write_burst<uint8_t>(std::span(bytes));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    return Err(Error::NoAvailablePhy);
}

[[nodiscard]] Result<void, BoschImu_Transport::Error> 
BoschImu_Transport::read_reg(const uint8_t reg_addr, uint8_t & data){
    // DEBUG_PRINTLN(std::hex, std::showbase, "read_reg", reg_addr, data);
    if(i2c_drv_){
        if(const auto res = i2c_drv_->read_reg(reg_addr, data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(reg_addr | 0x80), CONT);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_->read_single<uint8_t>(data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


    return Err(Error::NoAvailablePhy);
}

[[nodiscard]] Result<void, BoschImu_Transport::Error> 
BoschImu_Transport::read_burst(const uint8_t reg_addr, std::span<int16_t> pbuf){
    if(i2c_drv_){
        if(const auto res = (i2c_drv_->read_burst<int16_t>(reg_addr, pbuf, std::endian::little));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(reg_addr | 0x80), CONT);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_->read_burst<uint8_t>(
            std::span(reinterpret_cast<uint8_t *>(pbuf.data()), pbuf.size() * sizeof(int16_t)));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    return Err(Error::NoAvailablePhy);
}


[[nodiscard]] Result<void, InvensenseImu_Transport::Error> 
InvensenseImu_Transport::write_reg(const uint8_t reg_addr, const uint8_t data) {
    DEBUG_PRINTLN(std::hex, std::showbase, "write_reg", reg_addr, data);
    if(i2c_drv_){
        if(const auto res = i2c_drv_->write_reg(reg_addr, data); res.is_err())
            return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(reg_addr & 0x7f), CONT);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_->write_single<uint8_t>(data); res.is_err())
            return Err(res.unwrap_err());
        return Ok();
    }
    return Err(Error::NoAvailablePhy);
}

[[nodiscard]] Result<void, InvensenseImu_Transport::Error> 
InvensenseImu_Transport::read_reg(const uint8_t reg_addr, uint8_t & data) {
    DEBUG_PRINTLN(std::hex, std::showbase, "read_reg", reg_addr, data);
    if(i2c_drv_){
        if(const auto res = i2c_drv_->read_reg(reg_addr, data); res.is_err())
            return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(reg_addr | 0x80), CONT);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_->read_single<uint8_t>(data); res.is_err())
            return Err(res.unwrap_err());
        return Ok();
    }
    return Err(Error::NoAvailablePhy);
}

[[nodiscard]] Result<void, InvensenseImu_Transport::Error> 
InvensenseImu_Transport::read_burst(const uint8_t reg_addr, std::span<int16_t> pbuf){
    if(i2c_drv_){
        if(const auto res = i2c_drv_->read_burst<int16_t>(reg_addr, pbuf, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(reg_addr | 0x80), CONT);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_->read_burst<int16_t>(pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    return Err(Error::NoAvailablePhy);
}

[[nodiscard]] Result<void, StmicroImu_Transport::Error> StmicroImu_Transport::write_reg(
    const uint8_t reg_addr, const uint8_t data
) {
    if(i2c_drv_){
        if(const auto res = i2c_drv_->write_reg(uint8_t(reg_addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(reg_addr), CONT);
            res.is_err()) return Err(res.unwrap_err());

        if(const auto res = spi_drv_->write_single<uint8_t>(data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    return Err(StmicroImu_Transport::Error::NoAvailablePhy);
}

[[nodiscard]] Result<void, StmicroImu_Transport::Error> StmicroImu_Transport::read_reg(
    const uint8_t reg_addr, uint8_t & data
) {
    if(i2c_drv_){
        if(const auto res = i2c_drv_->read_reg(uint8_t(reg_addr), data);
            res.is_err()) return Err(res.unwrap_err());   
        return Ok();
    }else if(spi_drv_){
        if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(reg_addr) | 0x80), CONT);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_->read_single<uint8_t>(data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    return Err(StmicroImu_Transport::Error::NoAvailablePhy);
}

[[nodiscard]] Result<void, StmicroImu_Transport::Error> StmicroImu_Transport::read_burst(
    const uint8_t reg_addr, std::span<int16_t> pbuf
){
    if(i2c_drv_){
        if(const auto res = i2c_drv_->read_burst<int16_t>(uint8_t(reg_addr), pbuf, std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(reg_addr) | 0x80), CONT);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_->read_burst<int16_t>(pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    return Err(StmicroImu_Transport::Error::NoAvailablePhy);
}

[[nodiscard]] Result<void, StmicroImu_Transport::Error> StmicroImu_Transport::validate(){
    if(i2c_drv_){
        if(const auto res = i2c_drv_->validate();
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    if(spi_drv_){
        return Ok();
    }


    return Err(StmicroImu_Transport::Error::NoAvailablePhy);
}


[[nodiscard]] Result<void, ImuError> AsahiKaseiImu_Transport::write_reg(
    const uint8_t reg_addr, const uint8_t data
){
    if(i2c_drv_){
        if(const auto res = i2c_drv_->write_reg(uint8_t(reg_addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(reg_addr), CONT); 
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_->write_single<uint8_t>(data); 
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    return Err(ImuError::NoAvailablePhy);
}

[[nodiscard]] Result<void, ImuError> AsahiKaseiImu_Transport::read_reg(
    const uint8_t reg_addr, uint8_t & data
){
    if(i2c_drv_){
        if(const auto res = i2c_drv_->read_reg(uint8_t(reg_addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(reg_addr) | 0x80), CONT); 
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_->read_single<uint8_t>(data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    return Err(ImuError::NoAvailablePhy);
}

[[nodiscard]] Result<void, ImuError> AsahiKaseiImu_Transport::read_burst(
    const uint8_t reg_addr, std::span<int16_t> pbuf
){
    if(i2c_drv_){
        if(const auto res = i2c_drv_->read_burst<int16_t>(uint8_t(reg_addr), pbuf, std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(reg_addr) | 0x80), CONT);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_->read_burst<int16_t>(pbuf);
            res.is_err()) return Err(res.unwrap_err()); 
        return Ok();
    }

    return Err(ImuError::NoAvailablePhy);
}

[[nodiscard]] Result<void, ImuError> AsahiKaseiImu_Transport::read_burst(
    const uint8_t reg_addr, const std::span<uint8_t> pbuf
){
    TODO();
    return Ok();
}

[[nodiscard]] Result<void, ImuError> AsahiKaseiImu_Transport::validate(){
    TODO();
    return Ok();
}

[[nodiscard]] Result<void, ImuError> AnalogDeviceIMU_Transport::write_reg(
    const uint8_t reg_addr, const uint8_t reg_data
){
    if(i2c_drv_){
        if(const auto res = i2c_drv_->write_reg((uint8_t)((uint8_t)reg_addr & 0x7F), reg_data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    } else if (spi_drv_) {
        return Err(Error::SpiPhyIsNotImplementedYet);
    }
    return Err(ImuError::NoAvailablePhy);
}

[[nodiscard]] Result<void, ImuError> AnalogDeviceIMU_Transport::read_reg(
    const uint8_t reg_addr, uint8_t & reg_data
){
    if(i2c_drv_){
        if(const auto res = i2c_drv_->read_reg(uint8_t(reg_addr), reg_data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        return Err(Error::SpiPhyIsNotImplementedYet);
    }
    return Err(ImuError::NoAvailablePhy);
}

[[nodiscard]] Result<void, ImuError> AnalogDeviceIMU_Transport::read_burst(
    const uint8_t reg_addr, std::span<int16_t> pbuf
){
    if(i2c_drv_){
        if(const auto res = i2c_drv_->read_burst<int16_t>(uint8_t(reg_addr), pbuf, std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else if(spi_drv_){
        if(const auto res = spi_drv_->write_single<uint8_t>(uint8_t(uint8_t(reg_addr) | 0x80), CONT);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_->read_burst<int16_t>(pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    return Err(ImuError::NoAvailablePhy);
}
}    