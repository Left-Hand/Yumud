#include "sccb_drv.hpp"

using namespace ymd;
using namespace ymd::hal;

hal::HalResult SccbDrv::write_reg(const uint8_t reg_addr, const uint16_t reg_val){
    const auto guard = i2c_.create_guard();

    if(const auto res = i2c_.borrow(slave_addr_.with_write());
        res.is_err()) return res; 
        
    //#region 写入地址字节和第一个字节
    if(const auto res = i2c_.write(reg_addr);
        res.is_err()) return res;
    if(const auto res = i2c_.write(static_cast<uint8_t>(reg_val >> 8));
        res.is_err()) return res;
    //#endregion

    //#region 写入第二个字节
    if(const auto res = i2c_.borrow(slave_addr_.with_write());
        res.is_err()) return res;

    if(const auto res = i2c_.write(0xF0); 
        res.is_err()) return res;
    if(const auto res = i2c_.write(reg_val); 
        res.is_err()) return res;
    //#endregion

    return hal::HalResult::Ok();
}

hal::HalResult SccbDrv::read_reg(const uint8_t reg_addr, uint16_t & reg_val){
    const auto guard = i2c_.create_guard();

    if(const auto res = i2c_.borrow(slave_addr_.with_write());
        res.is_err()) return res;

    uint32_t data_l, data_h;

    // 写入地址字节
    if(const auto res = i2c_.write(reg_addr);
        res.is_err()) return res;

    // 写入第一个字节        
    if(const auto res = i2c_.borrow(slave_addr_.with_read());
        res.is_err()) return res;
    if(const auto res = i2c_.read(data_h, NACK);
        res.is_err()) return res;

    // 写入第二个字节
    if(const auto res = i2c_.borrow(slave_addr_.with_write());
        res.is_err()) return res;
    if(const auto res = i2c_.write(0xF0);
        res.is_err()) return res;

    if(const auto res = i2c_.borrow(slave_addr_.with_read());
        res.is_err()) return res;
    if(const auto res = i2c_.read(data_l, NACK);
        res.is_err()) return res;

    reg_val = static_cast<uint16_t>(static_cast<uint8_t>(data_h) << 8) 
        | static_cast<uint8_t>(data_l);

    return hal::HalResult::Ok();
}