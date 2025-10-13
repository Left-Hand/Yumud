#include "i2c.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::hal;

HalResult I2c::borrow(const I2cSlaveAddrWithRw req){
    if(false == owner_.is_borrowed()){
        owner_.borrow(req);
        return lead(req);
    }else if(owner_.is_borrowed_by(req)){
        owner_.borrow(req);
        return lead(req);
    }else{
        return hal::HalResult::OccuipedByOther;
    }
}
