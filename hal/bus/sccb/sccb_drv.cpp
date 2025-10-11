#include "sccb_drv.hpp"

using namespace ymd;
using namespace ymd::hal;

hal::HalResult SccbDrv::write_reg(const uint8_t addr, const uint16_t data){
    const auto guard = i2c_.create_guard();

    if(const auto res = i2c_.borrow(slave_addr_.with_write());
        res.is_err()) return res; 
        
    //#region 写入地址字节和第一个字节
    if(const auto err = 
        i2c_.write(addr)
        | i2c_.write(uint8_t(data >> 8));
    err.is_err()) return err;
    //#endregion

    //#region 写入第二个字节
    if(const auto err = i2c_.borrow(slave_addr_.with_write());
        err.is_err()) return err;


    if(const auto res = i2c_.write(0xF0); 
        res.is_err()) return res;
    if(const auto res = i2c_.write(data); 
        res.is_err()) return res;
    //#endregion


    return hal::HalResult::Ok();
}

hal::HalResult SccbDrv::read_reg(const uint8_t addr, uint16_t & data){
    const auto guard = i2c_.create_guard();

    if(const auto res = i2c_.borrow(slave_addr_.with_write());
        res.is_err()) return res;

    uint32_t data_l, data_h;

    // 写入地址字节
    i2c_.write(addr);

    // 写入第一个字节        
    i2c_.borrow(slave_addr_.with_read());
    i2c_.read(data_h, NACK);

    // 写入第二个字节
    i2c_.borrow(slave_addr_.with_write());
    i2c_.write(0xF0);

    i2c_.borrow(slave_addr_.with_read());
    i2c_.read(data_l, NACK);
    
    // i2c_.end();

    data = ((uint8_t)data_h << 8) | (uint8_t)data_l;

    return hal::HalResult::Ok();
}