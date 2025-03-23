#include "i2cdrv.hpp"

namespace ymd::hal{
BusError I2cDrv::release(){
    //避免上次总线的错误时序影响本次
    //例如使用软i2c时 传输到一半被强行关闭 此时总线有可能被钳位
    
    for(size_t i = 0; i < 4; i++){
        bus_.begin(index_);
        bus_.end();
    }
    bus_.begin(index_);
    return bus_.end();
}


BusError I2cDrv::verify(){
    {
        const auto guard = create_guard();
        if(const auto err = bus_.begin(index_ | 0x00); err.wrong()) return err;
    }
    
    return BusError::OK;
}

}