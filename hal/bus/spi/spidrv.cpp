#include "spidrv.hpp"
#include "core/clock/clock.hpp"

using namespace ymd;
using namespace ymd::hal;

hal::BusError SpiDrv::release(){
    if (auto err = spi_.begin(idx_.to_req()); err.is_ok()) {
        __nopn(4);
        spi_.end();
        return hal::BusError::Ok();
    }else{
        return err;
    }
}