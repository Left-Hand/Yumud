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

