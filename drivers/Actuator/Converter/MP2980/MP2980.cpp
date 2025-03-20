#include "MP2980.hpp"

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


#define WRITE_REG(reg) write_reg(reg.address, reg).unwrap();
#define READ_REG(reg) read_reg(reg.address, reg).unwrap();


MP2980 & MP2980::setFeedBackVref(const real_t vref){
    setFeedBackVrefMv(int(vref * 1000));
    return *this;
}

MP2980 & MP2980::setFeedBackVrefMv(const uint vref_mv){
    ref_reg.set(vref_mv);
    WRITE_REG(ref_reg);
    return *this;
}

// Enables power switching. 
// 1: Enable power switching 
// 0: Disable power switching but other internal control circuits work 
MP2980 & MP2980::enablePowerSwitching(const bool en){
    // The default value is determined via the ADDR pin setting. See Table 1 on 
    // page 28 for more details. 
    // If ENPWR resets from 1 to 0, the MP2980 resets the VREF bits to 0.5V 
    // (0011 1110 100). The MP2980 resets VREF one-time only to ensure that 
    // VREF can be changed to the required value via the digital interface after 
    // ENPWR = 0. The host must re-write VREF again if the system requires 
    // the previous VOUT value after ENPWR = 0. 
    // After ENPWER is set to 0, the discharge function works for 200ms. 
    ctrl1_reg.en_pwr = en;
    WRITE_REG(ctrl1_reg);
    return *this;
}

// Enables the VREF change function. 
// 1: VOUT changes based on the VREF registers. After VREF reaches the 
// new level set via the VREF bits, GO_BIT resets to 0 automatically 
// 0: VOUT cannot be changed 
MP2980 & MP2980::enableVrefChangeFunc(const bool en){
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
    WRITE_REG(ctrl1_reg);
    return *this;
}

MP2980 & MP2980::setPngState(const bool state){
    // Sets the power not good (PNG) status reset and control. See the PNG bit 
    // description on page 35. 
    // 1’b 0: The PNG bit status recovers to 0 once VOUT returns to its normal 
    // voltage range 
    // 1’b 1: The PNG bit status latches to 1 once VOUT exceeds the power good 
    // voltage range 
    ctrl1_reg.png_latch = state;
    WRITE_REG(ctrl1_reg);
    return *this;
}

MP2980 & MP2980::enableDither(const bool en){
    ctrl1_reg.dither = en;
    WRITE_REG(ctrl1_reg);
    return *this;
}

// enum class DischargePath{

// };

// MP2980 & setDischargePath(){

// }

MP2980 & MP2980::setVrefSlewRate(const VrefSlewRate slewrate){
    ctrl1_reg.sr = uint8_t(slewrate);
    WRITE_REG(ctrl1_reg);
    return *this;
}


MP2980 & MP2980::setOvpMode(const OvpMode mode){
    ctrl2_reg.ovp_mode = uint8_t(mode);
    WRITE_REG(ctrl2_reg);
    return *this;
}

enum class OcpMode:uint8_t{
    CycleByCycle,
    Hiccup, // default
    LatchOff
};

MP2980 & MP2980::setOcpMode(const OcpMode mode){
    ctrl2_reg.ocp_mode = uint8_t(mode);
    WRITE_REG(ctrl2_reg);
    return *this;
}


MP2980 & MP2980::setFsw(const Fsw fsw){
    ctrl2_reg.fsw = uint8_t(fsw);
    WRITE_REG(ctrl1_reg);
    return *this;
}

MP2980 & MP2980::setBuckBoostFsw(const BuckBoostFsw fsw){
    ctrl2_reg.bb_fsw = uint8_t(fsw);
    WRITE_REG(ctrl2_reg);
    return *this;
}

MP2980 & MP2980::setCurrLimitThreshold(const CurrLimitThreshold threshold){
    ilim_reg.ilim = uint8_t(threshold);
    WRITE_REG(ilim_reg);
    return *this;
}

MP2980::Interrupts MP2980::interrupts(){
    READ_REG(status_reg);
    return Interrupts(status_reg);
}

MP2980 & MP2980::setInterruptMask(const Interrupts mask){
    mask_reg = std::bit_cast<uint8_t>(mask);
    WRITE_REG(mask_reg);
    return *this;
}

MP2980 & MP2980::setOutputVolt(const real_t output_volt){
    const uint output_mv = int(output_volt * 1000);
    const uint fb_mv = (output_mv * fb_down_res_ohms) / (fb_up_res_ohms + fb_down_res_ohms);
    this->setFeedBackVrefMv(fb_mv);
    return *this;
}

MP2980 & MP2980::init(){
    auto & self = *this;
    self
        .setFeedBackVref(0.5_r)
        .enablePowerSwitching(true)
        .enableVrefChangeFunc(false)
        .setPngState(false)
        .enableDither(false)
        // .
        .setVrefSlewRate(VrefSlewRate::_50_V_S)
        .setCurrLimitThreshold(CurrLimitThreshold::_51_2_mV)
    ;

    return self;
}