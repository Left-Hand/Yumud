#include "TCA9548A.hpp"

using namespace ymd;
using namespace ymd::drivers;

void TCA9548A::switch_vbus(const uint8_t ch){
    if(last_ch_ == ch) return;
    const uint8_t temp = 1 << ch;
    self_i2c_drv_.write_blocks<>(temp, LSB).unwrap();
}

BusError TCA9548A::lead(const uint8_t address, const uint8_t ch){
    if((last_ch_ < 0) or (last_ch_ == ch)){
        last_ch_ = ch;//lock
        return i2c_.begin(address);
    }else{
        return BusError::OCCUPIED;
    }
}

void TCA9548A::trail(const uint8_t ch){
    if((last_ch_ >= 0) and (last_ch_ == ch)){
        i2c_.end();
        last_ch_ = -1;
        return ;
    }
}
