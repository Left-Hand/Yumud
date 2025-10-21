#include "si24r1.hpp"

#ifdef Si24R1_DEBUG
#define Si24R1_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define Si24R1_DEBUG(...)
#endif


using namespace ymd::drivers;
using namespace ymd;

using Error = Si24R1::Error;
template<typename T = void>
using IResult = Result<T, Error>;


IResult<size_t> Si24R1::available(){
    uint8_t size;
    if(const auto res = spi_drv_.transceive_single<uint8_t>(regs_.status_reg.as_ref(), 
        uint8_t(Command::R_RX_PL_WID), CONT); res.is_err()) 
        return Err(res.unwrap_err());
    if(const auto res = spi_drv_.read_single<uint8_t>(size); 
        res.is_err()) 
        return Err(res.unwrap_err());
    return Ok(size);
}

IResult<> Si24R1::clear_tx_fifo(){
    const auto res = spi_drv_.transceive_single(regs_.status_reg.as_ref(), 
        uint8_t(Command::FLUSH_TX));
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok();    
}

IResult<> Si24R1::clear_rx_fifo(){
    const auto res = spi_drv_.transceive_single(regs_.status_reg.as_ref(), 
        uint8_t(Command::FLUSH_RX));
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok();   
}

IResult<> Si24R1::update_status(){
    const auto res = spi_drv_.transceive_single(regs_.status_reg.as_ref(), uint8_t(Command::NOP));
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok();   
}