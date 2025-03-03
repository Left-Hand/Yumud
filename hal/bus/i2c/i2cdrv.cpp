#include "i2cdrv.hpp"

using namespace ymd;
using namespace ymd::hal;

BusError I2cDrv::release(){
    bus_.begin(index_);
    return bus_.end();
}

BusError I2cDrv::verify(){
    {
        const auto guard = createGuard();
        if(const auto err = bus_.begin(index_ | 0x00); err.wrong()) return err;
    }

    {
        const auto guard = createGuard();
        if(const auto err = bus_.begin(index_ | 0x01); err.wrong()) return err;
    }

    return BusError::OK;
}