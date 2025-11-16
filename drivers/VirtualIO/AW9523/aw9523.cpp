#include "aw9523.hpp"
#include "core/clock/clock.hpp"


using namespace ymd;
using namespace ymd::drivers;

// #define AW9523_DEBUG_EN

#ifdef AW9523_DEBUG_EN
#define AW9523_TODO(...) TODO()
#define AW9523_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define AW9523_PANIC(...) PANIC{__VA_ARGS__}
#define AW9523_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}


#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    PANIC{#x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define AW9523_DEBUG(...)
#define AW9523_TODO(...) PANIC_NSRC()
#define AW9523_PANIC(...)  PANIC_NSRC()
#define AW9523_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif

using Self = AW9523;

using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;

#define GUARD_NTH(nth)\
if(not is_index_valid(nth.count()))\
    return Err(Error::IndexOutOfRange);\

IResult<> Self::init(const Config & cfg){
    if(const auto res = reset();
        res.is_err()) return res;
    if(const auto res = validate();
        res.is_err()) return Err(res.unwrap_err());
    clock::delay(2ms);
    if(const auto res = set_led_current_limit(cfg.current_limit);
        res.is_err()) return res;

    for(size_t i = 0; i < MAX_CHANNELS; i++){
        const auto res = set_led_current_dutycycle(std::bit_cast<hal::PinNth>(
            hal::PinMask::from_nth(Nth(i)).as_u16()), 0);

        if(res.is_err()) [[unlikely]]
            return Err(res.unwrap_err());
    }
    regs_.led_mode_reg.mask = hal::PinMask::from_u16(0xffff);
    return Ok();
}


IResult<> Self::write_nth(const Nth nth, const BoolLevel data){
    GUARD_NTH(nth);
    buf_mask_ = buf_mask_.modify(nth, data);
    return write_by_mask(hal::PinMask(buf_mask_));
}

IResult<BoolLevel> Self::read_nth(const Nth nth){
    GUARD_NTH(nth);
    if(const auto res = read_mask();
        res.is_err()) return Err(res.unwrap_err());
    return Ok(BoolLevel::from(buf_mask_.test(nth)));
}

IResult<> Self::set_mode(const Nth nth, const hal::GpioMode mode){
    GUARD_NTH(nth);

    {
        auto reg = RegCopy(regs_.dir_reg);
        reg.mask = reg.mask.modify(nth, BoolLevel::from(mode.is_input()));
        
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    if(nth.count() < 8){
        auto reg = RegCopy(regs_.ctl_reg);
        reg.p0mod = mode.is_outpp();
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}

IResult<> Self::enable_irq(const Nth nth, const Enable en ){
    GUARD_NTH(nth);
    auto reg = RegCopy(regs_.inten_reg);
    reg.mask = reg.mask.modify(nth, BoolLevel::from(en == EN));
    return write_reg(reg);
}

IResult<> Self::enable_led_mode(const hal::PinMask pin_mask){
    auto reg = RegCopy(regs_.led_mode_reg);
    reg.mask = reg.mask | pin_mask;
    return write_reg(reg);
}

IResult<> Self::set_led_current_limit(const CurrentLimit limit){
    auto reg = RegCopy(regs_.ctl_reg);
    reg.isel = (uint8_t)limit;
    return write_reg(reg);
}

IResult<> Self::set_led_current_dutycycle(
    const hal::PinMask pin_mask, 
    const real_t dutycycle
){
    auto iter = pin_mask.iter();
    while(iter.has_next()){
        const auto nth = Nth(iter.index());
        if(const auto res = write_reg(get_dim_addr(nth), static_cast<uint8_t>(dutycycle * 255));
            res.is_err()) return Err(res.unwrap_err());
        iter.next();
    }
    return Ok();
}


IResult<> Self::validate(){
    auto & reg = regs_.chip_id_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return res;
    if(reg.id != VALID_CHIP_ID)
        return CHECK_ERR(Err(Error::WrongChipId), reg.id);
    return Ok();
}


IResult<> Self::write_reg(const RegAddr addr, const uint16_t data){
    // DEBUG_PRINTLN(uint8_t(addr), data);
    if(const auto res = i2c_drv_.write_reg(uint8_t(addr), data, std::endian::little);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}
