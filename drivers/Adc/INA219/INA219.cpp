#include "INA219.hpp"

#include "core/debug/debug.hpp"

using namespace ymd::drivers;
using namespace ymd::hal;
using namespace ymd;


#define INA219_DEBUG_EN 0
// #define INA219_DEBUG_EN 1

#if INA219_DEBUG_EN
#define INA219_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA219_PANIC(...) PANIC(__VA_ARGS__)
#define INA219_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define INA219_DEBUG(...)
#define INA219_PANIC(...)  PANIC_NSRC()
#define INA219_ASSERT(cond, ...) ASSERT_NSRC(cond)

#endif


using Error = INA219::Error;

template<typename T = void>
using IResult = Result<T, Error>;


IResult<> INA219::write_reg(const RegAddr addr, const uint16_t data){
    if(const auto res = i2c_drv_.write_reg(uint8_t(addr), data, std::endian::big);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> INA219::read_reg(const RegAddr addr, uint16_t & data){
    if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data, std::endian::big);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> INA219::read_reg(const RegAddr addr, int16_t & data){
    if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data, std::endian::big);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> INA219::read_burst(
    const RegAddr addr, 
    const std::span<uint16_t> pbuf
){
    // return i2c_drv_.read_burst(uint8_t(addr), p_std::span(data, len), std::endian::little);
    TODO();
    return Ok();
}



IResult<> INA219::update(){
    TODO();
    return Ok();
}


IResult<> INA219::validate(){
    // static constexpr uint16_t valid_manu_id = 0x5449;
    // static constexpr uint16_t valid_chip_id = 0x2260;
    
    INA219_ASSERT(i2c_drv_.validate().is_ok(), "INA219 i2c lost");

    // READ_REG(chipIDReg);
    // READ_REG(manufactureIDReg);

    // return INA219_ASSERT(
    //     (chipIDReg == valid_chip_id and manufactureIDReg == valid_manu_id), 
    //     "INA219 who am i failed", chipIDReg.as_bits(), manufactureIDReg.as_bits()
    // );
    return Ok();
}