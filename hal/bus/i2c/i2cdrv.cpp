#include "i2cdrv.hpp"

namespace ymd::hal{
hal::BusError I2cDrv::release(){
    //避免上次总线的错误时序影响本次
    //例如使用软i2c时 传输到一半被强行中止(MCU复位) 此时总线有可能被钳位
    
    for(size_t i = 0; i < 4; i++){
        i2c_.begin(slave_addr_.to_read_req());
        i2c_.end();
    }
    i2c_.begin(slave_addr_.to_read_req());
    return i2c_.end();
}


hal::BusError I2cDrv::verify(){
    const auto guard = i2c_.create_guard();
    if(const auto err = i2c_.begin(slave_addr_.to_read_req()); err.is_err()) return err;
    
    return hal::BusError::Ok();
}

}