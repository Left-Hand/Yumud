#include "INA219.hpp"

#include "sys/debug/debug.hpp"

using namespace ymd::drivers;
using namespace ymd::hal;
using namespace ymd;


#define INA219_DEBUG_ON 0
// #define INA219_DEBUG_ON 1

#if (INA219_DEBUG_ON == 1)
#define INA219_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA219_PANIC(...) PANIC(__VA_ARGS__)
#define INA219_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}

#define WRITE_REG(reg) this->writeReg(reg.address, reg.as_val()).loc().expect();
#define READ_REG(reg) this->readReg(reg.address, reg.as_ref()).loc().expect();
#else
#define INA219_DEBUG(...)
#define INA219_PANIC(...)  PANIC_NSRC()
#define INA219_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define WRITE_REG(reg) +this->writeReg(reg.address, reg.as_val());
#define READ_REG(reg) +this->readReg(reg.address, reg.as_ref());
#endif


using Error = INA219::Error;
// using BusResult = INA219::BusResult;


Result<void, Error> INA219::writeReg(const RegAddress addr, const uint16_t data){
    return Result<void, Error>(i2c_drv.writeReg(uint8_t(addr), data, MSB))
        .check_if<INA219_DEBUG_ON>("write error", uint8_t(addr), data);
}

Result<void, Error> INA219::readReg(const RegAddress addr, uint16_t & data){
    return Result<void, Error>(i2c_drv.readReg(uint8_t(addr), data, MSB))
        .check_if<INA219_DEBUG_ON>("read error", uint8_t(addr), data);
}

Result<void, Error> INA219::readReg(const RegAddress addr, int16_t & data){
    return Result<void, Error>(i2c_drv.readReg(uint8_t(addr), data, MSB))
        .check_if<INA219_DEBUG_ON>("read error", uint8_t(addr), data);
}

Result<void, Error> INA219::requestPool(const RegAddress addr, uint16_t * p_data, const size_t len){
    // return i2c_drv.readMulti(uint8_t(addr), p_std::span(data, len), LSB);
    return Ok();
}



Result<void, Error> INA219::update(){
    return Ok();
}


Result<void, Error> INA219::verify(){
    // scexpr uint16_t valid_manu_id = 0x5449;
    // scexpr uint16_t valid_chip_id = 0x2260;
    
    INA219_ASSERT(i2c_drv.verify().ok(), "INA219 i2c lost");

    // READ_REG(chipIDReg);
    // READ_REG(manufactureIDReg);

    // return INA219_ASSERT(
    //     (chipIDReg == valid_chip_id and manufactureIDReg == valid_manu_id), 
    //     "INA219 who am i failed", chipIDReg.as_val(), manufactureIDReg.as_val()
    // );
    return Ok();
}