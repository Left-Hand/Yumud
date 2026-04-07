#include "IST8310.hpp"

using namespace ymd;
using namespace ymd::drivers;

// https://zhuanlan.zhihu.com/p/588908861
// 在大疆的开发手册中找到了寄存器的手册


#ifdef IST8310_DEBUG
#undef IST8310_DEBUG
#define IST8310_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define IST8310_PANIC(...) PANIC(__VA_ARGS__)
#define IST8310_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define IST8310_DEBUG(...)
#define IST8310_PANIC(...)  PANIC_NSRC()
#define IST8310_ASSERT(cond, ...) ASSERT_NSRC(cond)
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

    if(const auto res = enable_continous(DISEN);
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
    std::array<int16_t, 3> buf;
    if(const auto res = read_bulk(regs_.axis_x_reg.REG_ADDR, std::span(buf));
        res.is_err()) return Err(res.unwrap_err());

    regs_.axis_x_reg.bits = buf[0];
    regs_.axis_y_reg.bits = buf[1];
    regs_.axis_z_reg.bits = buf[2];

    return Ok();
}

IResult<> IST8310::validate(){

    auto reg = Regs::R8_WhoAmI{};
    if(const auto res = read_reg(reg);
        res.is_err()) return res;
    if(reg.to_bits() != reg.KEY)
        return Err(Error::InvalidChipId);
    return Ok();
}

IResult<> IST8310::reset(){
    auto reg = RegCopy(regs_.ctrl2_reg);
    reg.reset = 1;
    const auto res = write_reg(reg);
    reg.reset = 0;
    return res;
}

IResult<> IST8310::enable_continous(const Enable en){
    auto reg = RegCopy(regs_.ctrl1_reg);
    reg.continous = (en == EN);
    return write_reg(reg);;
}


IResult<> IST8310::set_x_average_times(const AverageTimes times){
    auto reg = RegCopy(regs_.average_reg);
    reg.x_and_z_times = times;

    return write_reg(reg);
}


IResult<> IST8310::set_y_average_times(const AverageTimes times){
    auto reg = RegCopy(regs_.average_reg);

    reg.y_times = times;

    return write_reg(reg);
}



IResult<math::Vec3<iq24>> IST8310::read_mag(){
    auto conv = [](const int16_t data) -> iq16{
        return data * iq16(0.3);
    };

    return Ok{math::Vec3<iq24>{
        conv(std::bit_cast<int16_t>(regs_.axis_x_reg.to_bits())),
        conv(std::bit_cast<int16_t>(regs_.axis_y_reg.to_bits())),
        conv(std::bit_cast<int16_t>(regs_.axis_z_reg.to_bits()))
    }};
}

IResult<iq16> IST8310::get_temperature(){
    return Ok(regs_.temp_reg.temp_code.to_celsius());
}

IResult<bool> IST8310::is_data_ready(){
    
    auto reg = RegCopy(regs_.status1_reg);
    if(const auto res = read_reg(reg);
        res.is_err()) return (Err(res.unwrap_err()));
    return Ok(bool(reg.drdy));
}

IResult<> IST8310::enable_interrupt(const Enable en){
    auto reg = RegCopy(regs_.ctrl2_reg);
    reg.interrupt_en = (en == EN);
    return write_reg(reg);
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
    return Ok(bool(reg.interrupt_acting));
}