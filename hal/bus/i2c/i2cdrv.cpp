#include "i2cdrv.hpp"

namespace ymd::hal{
hal::HalResult I2cDrv::release(){
    //避免上次总线的错误时序影响本次
    //例如使用软i2c时 传输到一半被强行中止(MCU复位) 此时总线有可能被钳位
    
    for(size_t i = 0; i < 4; i++){
        // i2c_.borrow(slave_addr_.to_read_req());
        i2c_.lend();
    }

    // i2c_.borrow(slave_addr_.to_read_req());
    // i2c_.lend();

    return HalResult::Ok();
}


hal::HalResult I2cDrv::validate(){
    const auto res = i2c_.borrow(slave_addr_.to_read_req());
    i2c_.lend();
    return res;
}

}