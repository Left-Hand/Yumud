#include "MP2980.hpp"

using namespace ymd;
using namespace ymd::drivers;


#define MP2980_DEBUG

#ifdef MP2980_DEBUG
#undef MP2980_DEBUG
#define MP2980_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define MP2980_PANIC(...) PANIC(__VA_ARGS__)
#define MP2980_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define MP2980_DEBUG(...)
#define MP2980_PANIC(...)  PANIC()
#define MP2980_ASSERT(cond, ...) ASSERT(cond)
#endif



using Error = MP2980::Error;

template<typename T = void>
using IResult = Result<T, Error>;


IResult<> MP2980::set_feed_back_vref(const real_t vref){
    return set_feed_back_vref_mv(int(vref * 1000));
}

IResult<> MP2980::set_feed_back_vref_mv(const uint vref_mv){
    ref_reg.set(vref_mv);
    return write_reg(ref_reg);
}

// Enables power switching. 
// 1: Enable power switching 
// 0: Disable power switching but other internal control circuits work 
IResult<> MP2980::enable_power_switching(const bool en){
    // The default value is determined via the ADDR pin setting. See Table 1 on 
    // page 28 for more details. 
    // If ENPWR resets from 1 to 0, the MP2980 resets the VREF bits to 0.5V 
    // (0011 1110 100). The MP2980 resets VREF one-time only to ensure that 
    // VREF can be changed to the required value via the digital interface after 
    // ENPWR = 0. The host must re-write VREF again if the system requires 
    // the previous VOUT value after ENPWR = 0. 
    // After ENPWER is set to 0, the discharge function works for 200ms. 
    ctrl1_reg.en_pwr = en;
    return write_reg(ctrl1_reg);
}

// Enables the VREF change function. 
// 1: VOUT changes based on the VREF registers. After VREF reaches the 
// new level set via the VREF bits, GO_BIT resets to 0 automatically 
// 0: VOUT cannot be changed 
IResult<> MP2980::enable_vref_change_func(const bool en){
    // If GO_BIT = 1, enable the output change based on the VREF register. 
    // When the command completes (internal VREF steps to the target VREF), 
    // GO_BIT automatically resets to 0. This prevents false operation of VOUT 
    // scaling. 

    // Write the VREF registers (00h and 01h registers) first, then write GO_BIT 
    // = 1. VREF and VOUT change based on the new VREF. GO_BIT resets to 0 
    // when VREF reaches a new level. The host reads GO_BIT to determine 
    // whether the VREF scaling is finished. 
    // The VOUT discharge path is enabled when GO_BIT = 1, regardless of the 
    // DISCHG bit setting. This helps pull VOUT from high to low under light-load 
    // conditions. After GO_BIT resets to 0, the discharge continues and turn off 
    // after a 20ms delay. 

    ctrl1_reg.go_bit = en;
    return write_reg(ctrl1_reg);
}

IResult<> MP2980::set_png_state(const bool state){
    // Sets the power not good (PNG) status reset and control. See the PNG bit 
    // description on page 35. 
    // 1’b 0: The PNG bit status recovers to 0 once VOUT returns to its normal 
    // voltage range 
    // 1’b 1: The PNG bit status latches to 1 once VOUT exceeds the power good 
    // voltage range 
    ctrl1_reg.png_latch = state;
    return write_reg(ctrl1_reg);
}

IResult<> MP2980::enable_dither(const bool en){
    ctrl1_reg.dither = en;
    return write_reg(ctrl1_reg);
}

// enum class DischargePath{

// };

// IResult<> setDischargePath(){

// }

IResult<> MP2980::set_vref_slew_rate(const VrefSlewRate slewrate){
    ctrl1_reg.sr = uint8_t(slewrate);
    return write_reg(ctrl1_reg);
}


IResult<> MP2980::set_ovp_mode(const OvpMode mode){
    ctrl2_reg.ovp_mode = uint8_t(mode);
    return write_reg(ctrl2_reg);
}

enum class OcpMode:uint8_t{
    CycleByCycle,
    Hiccup, // default
    LatchOff
};

IResult<> MP2980::set_ocp_mode(const OcpMode mode){
    ctrl2_reg.ocp_mode = uint8_t(mode);
    return write_reg(ctrl2_reg);
}


IResult<> MP2980::set_fsw(const Fsw fsw){
    ctrl2_reg.fsw = uint8_t(fsw);
    return write_reg(ctrl1_reg);
}

IResult<> MP2980::set_buck_boost_fsw(const BuckBoostFsw fsw){
    ctrl2_reg.bb_fsw = uint8_t(fsw);
    return write_reg(ctrl2_reg);
}

IResult<> MP2980::set_curr_limit_threshold(const CurrLimitThreshold threshold){
    ilim_reg.ilim = uint8_t(threshold);
    return write_reg(ilim_reg);
}

IResult<MP2980::Interrupts> MP2980::interrupts(){
    if(const auto res = read_reg(status_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(Interrupts(status_reg));
}

IResult<> MP2980::set_interrupt_mask(const Interrupts mask){
    mask_reg = std::bit_cast<uint8_t>(mask);
    return write_reg(mask_reg);
}

IResult<> MP2980::set_output_volt(const real_t output_volt){
    const uint output_mv = int(output_volt * 1000);
    const uint fb_mv = (output_mv * fb_down_res_ohms) / (fb_up_res_ohms + fb_down_res_ohms);
    return set_feed_back_vref_mv(fb_mv);
}

IResult<> MP2980::init(){

    if(const auto res = set_feed_back_vref(0.5_r);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = enable_power_switching(true);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = enable_vref_change_func(false);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_png_state(false);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = enable_dither(false);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_vref_slew_rate(VrefSlewRate::_50_V_S);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_curr_limit_threshold(CurrLimitThreshold::_51_2_mV);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}