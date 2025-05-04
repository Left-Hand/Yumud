#include "ads1115.hpp"
#include "core/debug/debug.hpp"
#include "concept/analog_channel.hpp"

// #define ADS1115_DEBUG

#ifdef ADS1115_DEBUG
#undef ADS1115_DEBUG
#define ADS1115_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define ADS1115_PANIC(...) PANIC{__VA_ARGS__}
#define ADS1115_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#define READ_REG(reg) read_reg(reg.address, reg).loc().expect();
#define WRITE_REG(reg) write_reg(reg.address, reg).loc().expect();
#else
#define ADS1115_DEBUG(...)
#define ADS1115_PANIC(...)  PANIC_NSRC()
#define ADS1115_ASSERT(cond, ...) ASSERT_NSRC(cond)
#define READ_REG(reg) read_reg(reg.address, reg).expect();
#define WRITE_REG(reg) write_reg(reg.address, reg).expect();
#endif


namespace ymd::drivers{

using DeviceResult = ADS1115::DeviceResult;

DeviceResult ADS1115::read_reg(const RegAddress addr, uint16_t & data){
    return make_result(i2c_drv_.read_reg(uint8_t(addr), data, LSB)).check("read_reg failed");
}

DeviceResult ADS1115::write_reg(const RegAddress addr, const uint16_t data){
    return make_result(i2c_drv_.write_reg(uint8_t(addr), data, LSB)).check("write_reg failed");
}

DeviceResult ADS1115::validate(){
    return make_result(i2c_drv_.validate()).check("verify failed");
}
void ADS1115::start_conv(){
    auto & reg = config_reg;
    reg.busy = true;
    WRITE_REG(reg);
    reg.busy = false;
}

bool ADS1115::is_busy(){
    auto & reg = config_reg;
    READ_REG(reg);
    return reg.busy;
}

void ADS1115::set_threshold(int16_t low, int16_t high){
    low_thresh_reg.data = low;
    high_thresh_reg.data = high;
    WRITE_REG(low_thresh_reg);
    WRITE_REG(high_thresh_reg);
}

void ADS1115::enable_cont_mode(bool en){
    auto & reg = config_reg;
    reg.oneshot_en =!en;
    WRITE_REG(reg);
}

void ADS1115::set_pga(const PGA pga){
    auto & reg = config_reg;
    reg.pga = uint16_t(pga);
    WRITE_REG(reg);
}

void ADS1115::set_mux(const MUX mux){
    auto & reg = config_reg;
    reg.mux = uint16_t(mux);
    WRITE_REG(reg);
}

void ADS1115::set_data_rate(const DataRate data_rate){
    auto & reg = config_reg;
    reg.data_rate = uint16_t(data_rate);
    WRITE_REG(reg);
}

bool ADS1115::ready(){
    auto & reg = config_reg;
    READ_REG(reg);
    return reg.busy == false;
}

Option<real_t> ADS1115::result(){
    auto & reg = conversion_reg;
    if(read_reg(reg.address, reg).is_err()) return None;
    return Some(s16_to_uni(~reg.data) * 3.3_r);
    // return None;
}

}