//这个驱动还在推进状态

#pragma once

#include "si24r1_prelude.hpp"
namespace ymd::drivers{

class Si24R1:public Si24R1_Prelude{
public: 
    explicit Si24R1(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit Si24R1(hal::SpiDrv && spi_drv):
        spi_drv_(spi_drv){;}

    IResult<size_t> available();
private:

    hal::SpiDrv spi_drv_;
    Si24R1_Regset regs_ = {};

    IResult<> write_reg(RegAddr addr, const auto & value){
        addr &= ~uint8_t(Command::__RW_MASK);
        addr |= uint8_t(Command::W_REGISTER);
        spi_drv_.transceive_single(regs_.status_reg.as_bits_mut(), (addr), CONT);
        return spi_drv_.write_burst(&(value), sizeof(value));
    }

    IResult<> read_reg(RegAddr addr, auto & value){
        addr &= ~uint8_t(Command::__RW_MASK);
        addr |= uint8_t(Command::R_REGISTER);
        if(const auto res = spi_drv_.transceive_single(regs_.status_reg.as_bits_mut(), uint8_t(addr), CONT);
            res.is_err()) return Err(res.unwrap_err());
        
        if(const auto res = spi_drv_.read_burst(&(value), sizeof(value));
            res.is_err()) return Err(res.unwrap_err()); 

        return Ok();
    }

    IResult<> read_fifo(const std::span<uint8_t> pbuf){
        if(pbuf.size() == 0) return Ok();
        const uint8_t size = MIN(pbuf.size(), 32);
        if(const auto res = spi_drv_.transceive_single<uint8_t>(regs_.status_reg.as_bits_mut(), 
            uint8_t(Command::R_RX_PAYLOAD), CONT); res.is_err()) return Err(res.unwrap_err());
        
        if(const auto res = spi_drv_.read_burst<uint8_t>(std::span(pbuf.data(), size));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    
    }

    IResult<> write_fifo(const std::span<const uint8_t> pbuf){
        if(pbuf.size() == 0) return Ok();
        const uint8_t size = MIN(pbuf.size(), 32);
        if(const auto res = spi_drv_.transceive_single<uint8_t>(regs_.status_reg.as_bits_mut(), 
            uint8_t(Command::W_TX_PAYLOAD), CONT); res.is_err()) return Err(res.unwrap_err());

        if(const auto res = spi_drv_.write_burst<uint8_t>(std::span(pbuf.data(), size));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();

    }

    IResult<> write_fifo_no_ack(const std::span<const uint8_t> pbuf){
        if(pbuf.size() == 0) return Ok();
        const uint8_t size = MIN(pbuf.size(), 32);
        if(const auto res = spi_drv_.transceive_single<uint8_t>(regs_.status_reg.as_bits_mut(), 
            uint8_t(Command::W_TX_PAYLOAD_NO_ACK), CONT); res.is_err()) return Err(res.unwrap_err());
    
        if(const auto res = spi_drv_.write_burst<uint8_t>(std::span(pbuf.data(), size));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();

    }

    IResult<> clear_tx_fifo();

    IResult<> clear_rx_fifo();

    IResult<> update_status();

};

};