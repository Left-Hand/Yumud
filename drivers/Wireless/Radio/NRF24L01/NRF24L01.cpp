#include "NRF24L01.hpp"



// #define NRF24L01_DEBUG_EN

#ifdef NRF24L01_DEBUG_EN
#define NRF24L01_TODO(...) TODO()
#define NRF24L01_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define NRF24L01_PANIC(...) PANIC{__VA_ARGS__}
#define NRF24L01_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#define READ_REG(reg) readReg(reg.address, reg).loc().expect();
#define WRITE_REG(reg) writeReg(reg.address, reg).loc().expect();
#else
#define NRF24L01_DEBUG(...)
#define NRF24L01_TODO(...) PANIC_NSRC()
#define NRF24L01_PANIC(...)  PANIC_NSRC()
#define NRF24L01_ASSERT(cond, ...) ASSERT_NSRC(cond)
#define READ_REG(reg) readReg(reg.address, reg).unwrap()
#define WRITE_REG(reg) writeReg(reg.address, reg).unwrap()
#endif

using namespace ymd;
using namespace ymd::drivers;


using Error = NRF24L01::Error;

Result<void, Error> NRF24L01::write_command(const NRF24L01::Command cmd){
    return p_spi_drv_->writeSingle(uint8_t(cmd));
}

Result<void, Error> NRF24L01::write_reg(const uint8_t addr, const uint8_t data){
    return Ok();
}

Result<void, Error> NRF24L01::read_reg(const uint8_t addr, uint8_t & data){
    return Ok();
}

Result<size_t, Error> NRF24L01::transmit(std::span<std::byte> buf){

    return Ok(0u);
}

Result<size_t, Error> NRF24L01::receive(std::span<std::byte> buf){
    return Ok(0u);
}

// Result<_NRF24L01_Regs, Error> NRF24L01::dump(){
//     Regs regs = {
        
//     };

//     return Ok(regs);
// }