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

// #define PCA9685_VOLATILE_REFLASH

using namespace ymd;
using namespace ymd::drivers;

using Error = PCA9685::Error;
using Vport = PCA9685::PCA9685_Vport;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> PCA9685::set_frequency(uint32_t freq, iq16 trim){
    if(const auto res = read_reg(regs_.mode1_reg);
        res.is_err()) return res;
    
    {
        auto reg = RegCopy(regs_.mode1_reg);

        reg.sleep = true;
        if(const auto res = write_reg(reg); 
            res.is_err()) return res;
    }

    {
        auto reg = RegCopy(regs_.prescale_reg);
        reg.prescale = static_cast<uint8_t>((iq16(25000000.0 / 4096) / freq - 1) * trim);
        if(const auto res = write_reg(reg); 
            res.is_err()) return res;
    }

    {
        auto reg = RegCopy(regs_.mode1_reg);
        reg.sleep = false;
        
        if(const auto res = write_reg(reg);
            res.is_err()) return res;

        reg.as_bits_mut() = uint8_t(reg.to_bits() | uint8_t(0xa1));
        clock::delay(5ms);
        return write_reg(reg);
    }
}

IResult<> PCA9685::set_pwm(const Nth nth, uint16_t on, uint16_t off){
    if(nth.count() >= 16) return Err(Error::IndexOutOfRange);
    if(
        #ifdef PCA9685_VOLATILE_REFLASH
        true
        #else
        regs_.sub_channels[nth.count()].on.cvr != on
        #endif
    ){
        if(const auto res = write_reg(RegAddr(uint8_t(RegAddr::LED0_ON_L) + 4 * nth.count()), on); 
            res.is_err()) return res;
        regs_.sub_channels[nth.count()].on.cvr = on;
    }

    if(
        #ifdef PCA9685_VOLATILE_REFLASH
        true
        #else
        regs_.sub_channels[nth.count()].off.cvr != off
        #endif
    ){
        auto & reg = regs_.sub_channels[nth.count()].off;

        reg.full = false;
        reg.cvr = off;
        const auto address = RegAddr(uint8_t(RegAddr::LED0_OFF_L) + 4 * nth.count());
        if(const auto res = write_reg(address, reg.to_bits());
            res.is_err()) return res;
    }

    // PCA9685_DEBUG(off);
    return Ok();
}

IResult<> PCA9685::validate(){
    return reset();
}

IResult<> PCA9685::init(){
    if(const auto res = validate(); res.is_err()){
        PCA9685_PANIC("verify failed");
        return res;
    }

    clock::delay(1ms);

    auto reg = RegCopy(regs_.mode1_reg);
    if(const auto res = write_reg(reg);
        res.is_err()) return res;
    for(size_t i = 0; i < 16; i++){
        if(const auto res = set_pwm(Nth(i), 0, 0);
            res.is_err()) return res;
    }
    clock::delay(1ms);
    return Ok();
}


IResult<> PCA9685::set_sub_addr(const uint8_t index, const uint8_t addr){
    regs_.sub_addr_regs[index] = addr;
    return write_reg(
        RegAddr(uint8_t(RegAddr::SubAddr) + index), 
        regs_.sub_addr_regs[index]
    );
}

IResult<> PCA9685::reset(){

    if(const auto res = read_reg(regs_.mode1_reg);
        res.is_err()) return res;

    auto reg = RegCopy(regs_.mode1_reg);
    if(1 == reg.restart){
        reg.sleep = 0;
        if(const auto res = write_reg(reg);
            res.is_err()) return res;
    }
    clock::delay(1ms);
    reg.restart = 1;
    if(const auto res = write_reg(reg);
        res.is_err()) return res;

    reg.restart = 0;

    return Ok();
}

IResult<> PCA9685::enable_ext_clk(const Enable en){
    auto reg = RegCopy(regs_.mode1_reg);
    reg.extclk = en == EN;
    return write_reg(reg);
}

IResult<> PCA9685::enable_sleep(const Enable en){
    auto reg = RegCopy(regs_.mode1_reg);
    reg.sleep = en == EN;
    return write_reg(reg);
}
