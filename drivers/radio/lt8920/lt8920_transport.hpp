#pragma once

#include "lt8920_prelude.hpp"

namespace ymd::drivers{

class LT8920_TransportIntf: public LT8920_Prelude{
public:
    using FlagReg = LT8920_Regset::R16_Flag;
    virtual IResult<> write_reg(FlagReg & flag_reg, const uint8_t reg_addr, const uint16_t reg_val) = 0;
    virtual IResult<> read_reg(FlagReg & flag_reg, const uint8_t reg_addr, uint16_t &reg_val) = 0;

    virtual IResult<> write_fifo(std::span<const uint8_t> bytes) = 0;
    virtual IResult<> read_fifo(std::span<uint8_t> bytes) = 0;
    virtual IResult<> update_fifo_status(FlagReg & flag_reg) = 0;
};


struct LT8920_SpiTransport final : public LT8920_TransportIntf{
private:
    hal::SpiDrv spi_drv_;
public:
    explicit LT8920_SpiTransport(const hal::SpiDrv & spi_drv) : 
        spi_drv_(spi_drv) {;}
    explicit LT8920_SpiTransport(hal::SpiDrv && spi_drv) : 
        spi_drv_(std::move(spi_drv)) {;}
    explicit LT8920_SpiTransport(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank) : 
        spi_drv_(hal::SpiDrv(spi, rank)) {;}

    IResult<> write_reg(FlagReg & flag_reg, const uint8_t reg_addr, const uint16_t reg_val);

    IResult<> read_reg(FlagReg & flag_reg, const uint8_t reg_addr, uint16_t & reg_val);


    IResult<> write_fifo(std::span<const uint8_t> bytes){
        if(const auto res = spi_drv_.write_single<uint8_t>(uint8_t(50), CONT); 
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_.write_bulk<uint8_t>(bytes);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_fifo(std::span<uint8_t> bytes){
        if(const auto res = spi_drv_.write_single<uint8_t>(uint8_t(50 | 0x80), CONT); 
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_.read_bulk<uint8_t>(bytes);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> update_fifo_status(FlagReg & flag_reg){
        (void)flag_reg;
        // return spi_drv_->transceive_single(flag_reg.as_bits_mut(), flag_reg.reg_addr);
        TODO();
    }

private:
    [[maybe_unused]] static void delayT3(){clock::delay(1us);}

    [[maybe_unused]] static void delayT5(){clock::delay(1us);}
};

struct LT8920_I2cTransport final : public LT8920_TransportIntf{
private:
    hal::I2cDrv i2c_drv_;
public:
    explicit LT8920_I2cTransport(const hal::I2cDrv & i2c_drv) : 
        i2c_drv_(i2c_drv) {;}
    explicit LT8920_I2cTransport(hal::I2cDrv && i2c_drv) : 
        i2c_drv_(std::move(i2c_drv)) {;}


    IResult<> write_reg(FlagReg & flag_reg, const uint8_t reg_addr, const uint16_t reg_val){
        (void)flag_reg;
        if(const auto res = i2c_drv_.write_reg(uint8_t(reg_addr), reg_val, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }


    IResult<> read_reg(FlagReg & flag_reg, const uint8_t reg_addr, uint16_t & reg_val){
        (void)flag_reg;
        if(const auto res = i2c_drv_.read_reg(uint8_t(reg_addr), reg_val, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();

    }

    IResult<> write_fifo(std::span<const uint8_t> bytes){
        if(const auto res = i2c_drv_.write_bulk(uint8_t(50) , bytes);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(); 
    }

    IResult<> read_fifo(std::span<uint8_t> bytes){
        if(const auto res = i2c_drv_.read_bulk(uint8_t(50), bytes);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();

    }

    IResult<> update_fifo_status(FlagReg & flag_reg){
        if(const auto res = read_reg(flag_reg, flag_reg.REG_ADDR, flag_reg.as_bits_mut());
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
};
}