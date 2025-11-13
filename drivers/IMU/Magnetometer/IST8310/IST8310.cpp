#include "IST8310.hpp"

using namespace ymd;
using namespace ymd::drivers;

#ifdef IST8310_DEBUG
#undef IST8310_DEBUG
#define IST8310_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define IST8310_PANIC(...) PANIC(__VA_ARGS__)
#define IST8310_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define IST8310_DEBUG(...)
#define IST8310_PANIC(...)  PANIC()
#define IST8310_ASSERT(cond, ...) ASSERT(cond)
#endif


using Error = ImuError;

template<typename T = void>
using IResult= Result<T, Error>;

IResult<> IST8310::init(){
    if(const auto res = reset();
        res.is_err()) return res;

    clock::delay(10ms);

    if(const auto res = validate();
        res.is_err()) return res;

    if(const auto res = enable_contious(DISEN);
        res.is_err()) return res;
    if(const auto res = set_x_average_times(AverageTimes::_4);
        res.is_err()) return res;
    if(const auto res = set_y_average_times(AverageTimes::_4);
        res.is_err()) return res;
    if(const auto res = update();
        res.is_err()) return res;
    return Ok();
}
IResult<> IST8310::update(){
    auto & reg = regs_.axis_x_reg;
    return read_burst(reg.ADDRESS, &reg.as_ref(), 3);
}

IResult<> IST8310::validate(){
    auto reg = RegCopy(regs_.whoami_reg);
    if(const auto res = read_reg(reg);
        res.is_err()) return res;
    if(reg.as_val() != reg.expected_value)
        return Err(Error::InvalidChipId);
    return Ok();
}

IResult<> IST8310::reset(){
    auto reg = RegCopy(regs_.ctrl2_reg);
    reg.reset = 1;
    return write_reg(reg);
    reg.reset = 0;
}

IResult<> IST8310::enable_contious(const Enable en){
    auto reg = RegCopy(regs_.ctrl1_reg);
    reg.cont = en == EN;
    return write_reg(reg);;
}


IResult<> IST8310::set_x_average_times(const AverageTimes times){
    auto reg = RegCopy(regs_.average_reg);
    reg.x_times = times;

    return write_reg(reg);
}


IResult<> IST8310::set_y_average_times(const AverageTimes times){
    auto reg = RegCopy(regs_.average_reg);

    reg.y_times = times;

    return write_reg(reg);
}



IResult<Vec3<iq24>> IST8310::read_mag(){
    auto conv = [](const int16_t data) -> real_t{
        return data * real_t(0.3);
    };

    return Ok{Vec3<iq24>{
        conv(regs_.axis_x_reg.as_val()),
        conv(regs_.axis_y_reg.as_val()),
        conv(regs_.axis_z_reg.as_val())
    }};
}

IResult<iq16> IST8310::get_temperature(){
    return Ok(regs_.temp_reg.to_temp());
}

IResult<bool> IST8310::is_data_ready(){
    
    auto reg = RegCopy(regs_.status1_reg);
    if(const auto res = read_reg(reg);
        res.is_err()) return (Err(res.unwrap_err()));
    return Ok(bool(reg.drdy));
}

IResult<> IST8310::enable_interrupt(const Enable en){
    auto reg = RegCopy(regs_.ctrl2_reg);
    reg.int_en = en == EN;
    return write_reg(reg);;
}

IResult<> IST8310::set_interrupt_level(const BoolLevel lv){
    auto reg = RegCopy(regs_.ctrl2_reg);
    reg.drdy_level = lv.to_bool();
    return write_reg(reg);;
}

IResult<> IST8310::enable_sleep(const Enable en){
    auto reg = RegCopy(regs_.ctrl1_reg);
    reg.awake = en == DISEN;
    return write_reg(reg);;
}

IResult<bool> IST8310::get_interrupt_status(){
    auto reg = RegCopy(regs_.status2_reg);
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(reg.on_int));
}