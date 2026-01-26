#include "BMI088.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = ImuError;

template<typename T = void>
using IResult = Result<T, Error>;

template<typename Fn, typename Fn_Dur>
IResult<> retry(const size_t times, Fn && fn, Fn_Dur && fn_dur){
    if constexpr(!std::is_null_pointer_v<Fn_Dur>) std::forward<Fn_Dur>(fn_dur)();
    IResult<> res = std::forward<Fn>(fn)();
    if(!times) return res;
    else return retry(times - 1, std::forward<Fn>(fn), std::forward<Fn_Dur>(fn_dur));
}

template<typename Fn>
IResult<> retry(const size_t times, Fn && fn){
    return retry(times, std::forward<Fn>(fn), nullptr);
}


static constexpr auto MAX_RETRY_TIMES = 3u;

static constexpr uint8_t ACC_CHIP_ID = 0;


IResult<> BMI088_Acc::init(){
    TODO();
    return Ok();
}


IResult<> BMI088_Acc::reset(){
    return transport_.write_command(0xb6);
}

IResult<> BMI088_Acc::verify_chip_id(){
    auto & reg = regs_.acc_chipid_reg;
    if(const auto res = transport_.read_regs(reg);
        res.is_err()) return res;

    if(reg.bits != ACC_CHIP_ID)
        Err(Error::InvalidChipId);

    return Ok();
}

IResult<> BMI088_Acc::validate(){
    if(const auto res = reset();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = transport_.validate();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = retry(MAX_RETRY_TIMES, [&]{return verify_chip_id();}, [](){clock::delay(1ms);});
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_acc_odr(AccOdr::_200Hz);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_acc_bwp(AccBwp::Normal);
        res.is_err()) return Err(res.unwrap_err());
    // if(const auto res = interrupts[0].enable_output(EN);
    //     res.is_err()) return Err(res.unwrap_err());
    // if(const auto res = interrupts[1].enable_output(EN);
    //     res.is_err()) return Err(res.unwrap_err());
    // if(const auto res = retry(MAX_RETRY_TIMES, [&]{return verifyChipId();}, [](){clock::delay(1ms);});
    //     res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> BMI088_Acc::update(){
    auto & reg = regs_.acc_x_reg;
    return transport_.read_burst(
        reg.ADDRESS, 
        std::span(
            reinterpret_cast<int16_t *>(&(reg.as_bits_mut())), 
            3
        )
    );
}


IResult<Vec3<iq24>> BMI088_Acc::read_acc(){
    return Ok(Vec3<iq24>(
        std::bit_cast<int16_t>(regs_.acc_x_reg.to_bits()) * acc_scaler_,
        std::bit_cast<int16_t>(regs_.acc_y_reg.to_bits()) * acc_scaler_,
        std::bit_cast<int16_t>(regs_.acc_z_reg.to_bits()) * acc_scaler_
    ));
}

static constexpr Option<iq16> temp_bits_to_celsius(uint8_t msb, uint8_t lsb){

    if(msb == 0x80) return None;

	const uint16_t bits_u11 = uint16_t((static_cast<uint32_t>(msb) << 3) + (static_cast<uint32_t>(lsb) >> 5));

    const int16_t temp_i11 = [&] -> int16_t{
        if(bits_u11 > 1023) 
            return static_cast<int16_t>(bits_u11) - 2048;
        else
            return static_cast<int16_t>(bits_u11);
    }();

    const auto ret = iq16(iq16(temp_i11) >> 3) + 23;
    return Some(ret);   
}

// static constexpr auto d = 0x3e*8;
// static constexpr auto d2 = (d >> 3) + 23;

static_assert(temp_bits_to_celsius(0xc1,0x00).unwrap() == -40);
static_assert(temp_bits_to_celsius(0x3e,0x00).unwrap() == 85);
static_assert(temp_bits_to_celsius(0x80,0x00).is_none());
IResult<iq16> BMI088_Acc::read_temp(){

    const auto may_temp = temp_bits_to_celsius(regs_.temp_msb_reg.bits, regs_.temp_lsb_reg.bits);
    if(may_temp.is_some())
        return Ok(may_temp.unwrap());
    else
        return Err(Error::InvalidTemperature);
}

IResult<> BMI088_Acc::set_acc_fs(const AccFs fs){
    auto & reg = regs_.acc_range_reg;
    acc_scaler_ = calculate_acc_scale(fs);
    reg.acc_range = uint8_t(fs);
    return transport_.write_regs(reg);
}


IResult<> BMI088_Acc::set_acc_bwp(const AccBwp bwp){
    auto & reg = regs_.acc_conf_reg;
    reg.acc_bwp = uint8_t(bwp);
    return transport_.write_regs(reg);
}


IResult<> BMI088_Acc::set_acc_odr(const AccOdr odr){
    auto & reg = regs_.acc_conf_reg;
    reg.acc_odr = uint8_t(odr);
    return transport_.write_regs(reg);
}