#include "TCA9548A.hpp"

using namespace ymd;
using namespace ymd::drivers;

hal::BusError TCA9548A::switch_vbus(const uint8_t ch){
    if((last_ch_.is_some()) and (last_ch_.unwrap() == ch)) return hal::BusError::OccuipedByOther;
    const uint8_t temp = 1 << ch;
    return self_i2c_drv_.write_blocks<>(temp, LSB);
}

hal::BusError TCA9548A::lead(const uint8_t address, const uint8_t ch){
    if((last_ch_.is_none()) 
        or (
            (last_ch_.is_some()) 
            and (last_ch_.unwrap() == ch)
        )){
        last_ch_ = Some(ch);//lock
        return i2c_.begin(hal::LockRequest{address, 0});
    }else{
        return hal::BusError::OccuipedByOther;
    }
}

void TCA9548A::trail(const uint8_t ch){
    if((last_ch_.is_some()) and (last_ch_.unwrap() == ch)){
        i2c_.end();
        last_ch_ = None;
        return ;
    }
}
