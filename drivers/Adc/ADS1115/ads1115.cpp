#include "ads1115.hpp"
#include "sys/debug/debug.hpp"


#define ADS1115_DEBUG

#ifdef ADS1115_DEBUG
#undef ADS1115_DEBUG
#define ADS1115_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define ADS1115_PANIC(...) PANIC{__VA_ARGS__}
#define ADS1115_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define ADS1115_DEBUG(...)
#define ADS1115_PANIC(...)  PANIC{}
#define ADS1115_ASSERT(cond, ...) ASSERT{cond}
#endif
namespace ymd::drivers{


BusResult ADS1115::readReg(const RegAddress addr, uint16_t & data){
    return make_result(i2c_drv_.readReg(uint8_t(addr), data, LSB)).check("readReg failed");
}

BusResult ADS1115::writeReg(const RegAddress addr, const uint16_t data){
    return make_result(i2c_drv_.writeReg(uint8_t(addr), data, LSB)).check("writeReg failed");
}

BusResult ADS1115::verify(){
    return make_result(i2c_drv_.verify()).check("verify failed");
}
void ADS1115::startConv(){
    auto & reg = config_reg;
    reg.busy = true;
    writeReg(reg.address, (reg));
    reg.busy = false;
}

bool ADS1115::isBusy(){
    auto & reg = config_reg;
    readReg(reg.address, (reg));
    return reg.busy;
}

void ADS1115::setThreshold(int16_t low, int16_t high){
    low_thresh_reg.data = low;
    high_thresh_reg.data = high;
    writeReg(LowThreshReg::address, (low_thresh_reg));
    writeReg(HighThreshReg::address, (high_thresh_reg));
}

void ADS1115::enableContMode(bool en){
    auto & reg = config_reg;
    reg.oneshot_en =!en;
    writeReg(reg.address, (reg));
}

void ADS1115::setPga(const PGA pga){
    auto & reg = config_reg;
    reg.pga = uint16_t(pga);
    writeReg(reg.address, (reg));
}

void ADS1115::setMux(const MUX mux){
    auto & reg = config_reg;
    reg.mux = uint16_t(mux);
    writeReg(reg.address, (reg));
}

void ADS1115::setDataRate(const DataRate data_rate){
    auto & reg = config_reg;
    reg.data_rate = uint16_t(data_rate);
    writeReg(reg.address, (reg));
}

bool ADS1115::ready(){
    auto & reg = config_reg;
    readReg(reg.address, (reg));
    return reg.busy == false;
}

Option<real_t> ADS1115::result(){
    auto & reg = conversion_reg;
    if(readReg(reg.address, (reg)).is_err()) return None;
    return Some(s16_to_uni(~reg.data) * 3.3_r);
    // return None;
}

}