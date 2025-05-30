#include "pca9685.hpp"
#include "core/debug/debug.hpp"



#define PCA9685_DEBUG_EN

#ifdef PCA9685_DEBUG_EN
#define PCA9685_TODO(...) TODO()
#define PCA9685_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define PCA9685_PANIC(...) PANIC{__VA_ARGS__}
#define PCA9685_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define PCA9685_DEBUG(...)
#define PCA9685_TODO(...) PANIC_NSRC()
#define PCA9685_PANIC(...)  PANIC_NSRC()
#define PCA9685_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

// #define PCA9685_FORCEWRITE

using namespace ymd;
using namespace ymd::drivers;

using Error = PCA9685::Error;
using Vport = PCA9685::PCA9685_Vport;

void Vport::write_mask(const hal::PinMask mask){
    TODO();
}

hal::PinMask Vport::read_mask(){
    TODO();
    return hal::PinMask::from_index(0);
}

Result<void, Error> PCA9685::set_frequency(uint freq, real_t trim){
    if(const auto res = read_reg(RegAddress::Mode1, mode1_reg);
        res.is_err()) return res;
    
    prescale_reg = int((real_t(25000000.0 / 4096) / freq - 1) * trim);
    mode1_reg.sleep = true;
    if(const auto res = write_reg(RegAddress::Mode1, mode1_reg); 
        res.is_err()) return res;
    if(const auto res = write_reg(RegAddress::Prescale, prescale_reg); 
        res.is_err()) return res;
    mode1_reg.sleep = false;
    
    if(const auto res = write_reg(RegAddress::Mode1, mode1_reg);
        res.is_err()) return res;
    clock::delay(5ms);

    mode1_reg = uint8_t(mode1_reg | uint8_t(0xa1));
    return write_reg(RegAddress::Mode1, mode1_reg);
}

Result<void, Error> PCA9685::set_pwm(uint8_t channel, uint16_t on, uint16_t off){
    if(channel > 15) return Err(Error::IndexOutOfRange);
    
    if(
        #ifdef PCA9685_FORCEWRITE
        true
        #else
        sub_channels[channel].on.cvr != on
        #endif
    ){
        if(const auto res = write_reg(RegAddress(uint8_t(RegAddress::LED0_ON_L) + 4 * channel), on); 
            res.is_err()) return res;
        sub_channels[channel].on.cvr = on;
    }

    if(
        #ifdef PCA9685_FORCEWRITE
        true
        #else
        sub_channels[channel].off.cvr != off
        #endif
    ){
        auto & reg = sub_channels[channel].off;

        reg.full = false;
        reg.cvr = off;
        if(const auto res = write_reg(RegAddress(uint8_t(RegAddress::LED0_OFF_L) + 4 * channel), reg);
            res.is_err()) return res;
    }

    // PCA9685_DEBUG(off);
    return Ok();
}

Result<void, Error> PCA9685::validate(){
    return reset();
}

Result<void, Error> PCA9685::init(){
    if(const auto res = validate(); res.is_err()){
        PCA9685_PANIC("verify failed");
        return res;
    }
    return Ok();

    clock::delay(1ms);
    mode1_reg = 0;
    if(const auto res = write_reg(RegAddress::Mode1, mode1_reg);
        res.is_err()) return res;
    for(size_t i = 0; i < 16; i++){
        if(const auto res = set_pwm(i, 0, 0);
            res.is_err()) return res;
    }
    clock::delay(1ms);
    return Ok();
}


Result<void, Error> PCA9685::set_sub_addr(const uint8_t index, const uint8_t addr){
    sub_addr_regs[index] = addr;
    return write_reg(RegAddress(uint8_t(RegAddress::SubAddr) + index), sub_addr_regs[index]);
}

Result<void, Error> PCA9685::reset(){

    if(const auto res = read_reg(RegAddress::Mode1, mode1_reg);
        res.is_err()) return res;
    if(1 == mode1_reg.restart){
        mode1_reg.sleep = 0;
        if(const auto res = write_reg(RegAddress::Mode1, mode1_reg);
            res.is_err()) return res;
    }
    clock::delay(1ms);
    mode1_reg.restart = 1;
    if(const auto res = write_reg(RegAddress::Mode1, mode1_reg);
        res.is_err()) return res;

    mode1_reg.restart = 0;

    return Ok();
}

Result<void, Error> PCA9685::enable_ext_clk(const Enable en){
    mode1_reg.extclk = en == EN;
    return write_reg(RegAddress::Mode1, mode1_reg);
}

Result<void, Error> PCA9685::enable_sleep(const Enable en){
    mode1_reg.sleep = en == EN;
    return write_reg(RegAddress::Mode1, mode1_reg);
}

void Vport::set_by_mask(const hal::PinMask mask){
    TODO();
    // buf |= mask;
    // write(buf);
}

void Vport::clr_by_mask(const hal::PinMask mask){
    TODO();
    // buf &= ~mask;
    // write(buf);
}

void Vport::write_by_mask(const hal::PinMask mask){
    TODO();
    // buf &= ~mask;
    // write(buf);
}

void Vport::write_by_index(const size_t index, const BoolLevel level){
    TODO();
    // if(!isIndexValid(index))return;
    // if(data) buf |= 1 << index;
    // else buf &= ~(1 << index);
    // write(buf);
}

BoolLevel Vport::read_by_index(const size_t index){
    TODO();
    // if(!isIndexValid(index)) return false;
    // read();
    // return (buf & (1 << index));
    return LOW;
}


void Vport::set_mode(const size_t index, const hal::GpioMode mode){
    TODO();
//     if(!isIndexValid(index))return;
//     uint16_t mask = 1 << index;
//     if(GpioMode::isIn(mode)) dir |= mask;
//     else dir &= ~mask;
//     write_reg(RegAddress::dir, dir);

//     if(index < 8){
//         ctl.p0mod = GpioMode::isPP(mode);
//         write_reg(RegAddress::ctl, ctl.data);
//     }
}


__fast_inline BoolLevel PCA9685::PCA9685Channel::read() const {
    TODO();
    return LOW;
}
