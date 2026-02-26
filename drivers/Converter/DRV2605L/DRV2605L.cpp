#include "DRV2605L.hpp"

#include "core/debug/debug.hpp"

#define DRV2605_DEBUG_EN 0

#if DRV2605_DEBUG_EN

#define DRV2605_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define DRV2605_PANIC(...) PANIC{__VA_ARGS__}
#define DRV2605_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define DRV2605_DEBUG(...)
#define DRV2605_PANIC(...)  PANIC_NSRC()
#define DRV2605_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif


using namespace ymd;
using namespace ymd::drivers;

using Error = DRV2605L::Error;
using RegAddr = DRV2605L::RegAddr;

static constexpr auto STATUS_Reg          = RegAddr::Status;
static constexpr auto MODE_Reg            = RegAddr::Mode;
static constexpr auto RTP_INPUT_Reg       = RegAddr::RtpInput;
static constexpr auto LIB_SEL_Reg         = RegAddr::LibSel;
static constexpr auto WAV_SEQ1_Reg        = RegAddr::WavSEQ1;
static constexpr auto WAV_SEQ2_Reg        = RegAddr::WavSEQ2;
static constexpr auto WAV_SEQ3_Reg        = RegAddr::WavSEQ3;
static constexpr auto WAV_SEQ4_Reg        = RegAddr::WavSEQ4;
static constexpr auto WAV_SEQ5_Reg        = RegAddr::WavSEQ5;
static constexpr auto WAV_SEQ6_Reg        = RegAddr::WavSEQ6;
static constexpr auto WAV_SEQ7_Reg        = RegAddr::WavSEQ7;
static constexpr auto WAV_SEQ8_Reg        = RegAddr::WavSEQ8;
static constexpr auto GO_Reg              = RegAddr::Go;
static constexpr auto ODT_OFFSET_Reg      = RegAddr::OdtOffset;
static constexpr auto SPT_Reg             = RegAddr::Spt;
static constexpr auto SNT_Reg             = RegAddr::Snt;
static constexpr auto BRT_Reg             = RegAddr::Brt;
static constexpr auto ATV_CON_Reg         = RegAddr::AtvCon;
static constexpr auto ATV_MIN_IN_Reg      = RegAddr::AtvMinIn;
static constexpr auto ATV_MAX_IN_Reg      = RegAddr::AtvMaxIn;
static constexpr auto ATV_MIN_OUT_Reg     = RegAddr::AtvMinOut;
static constexpr auto ATV_MAX_OUT_Reg     = RegAddr::AtvMaxOut;
static constexpr auto RATED_VOLTAGE_Reg   = RegAddr::RatedVoltage;
static constexpr auto OD_CLAMP_Reg        = RegAddr::OdClamp;
static constexpr auto A_CAL_COMP_Reg      = RegAddr::ACalComp;
static constexpr auto A_CAL_BEMF_Reg      = RegAddr::ACalBemf;
static constexpr auto FB_CON_Reg          = RegAddr::FbCon;
static constexpr auto CONTRL1_Reg         = RegAddr::Control1;
static constexpr auto CONTRL2_Reg         = RegAddr::Control2;
static constexpr auto CONTRL3_Reg         = RegAddr::Control3;
static constexpr auto CONTRL4_Reg         = RegAddr::Control4;
static constexpr auto VBAT_MON_Reg        = RegAddr::VbatMon;
static constexpr auto LRA_RESON_Reg       = RegAddr::LraReson;



Result<void, Error> DRV2605L::set_fb_brake_factor(const FbBrakeFactor factor){
    auto reg = RegCopy(feedback_control_reg);
    reg.fb_brake_factor = uint8_t(factor);
    return write_reg(reg);
}

Result<void, Error> DRV2605L::set_fb_brake_factor(const int fractor){
    switch(fractor){
        case 1:     return set_fb_brake_factor(FbBrakeFactor::_1x);
        case 2:     return set_fb_brake_factor(FbBrakeFactor::_2x);
        case 3:     return set_fb_brake_factor(FbBrakeFactor::_3x);
        case 4:     return set_fb_brake_factor(FbBrakeFactor::_4x);
        case 6:     return set_fb_brake_factor(FbBrakeFactor::_6x);
        case 8:     return set_fb_brake_factor(FbBrakeFactor::_8x);
        case 16:    return set_fb_brake_factor(FbBrakeFactor::_16x);
        default:
            DRV2605_PANIC("invalid brake factor");
    }
}

Result<void, Error> DRV2605L::set_bemf_gain(const BemfGain gain){
    auto reg = RegCopy(feedback_control_reg);
    reg.bemf_gain = uint8_t(gain);
    return write_reg(reg);
}

Result<void, Error> DRV2605L::set_loop_gain(const LoopGain gain){
    auto reg = RegCopy(feedback_control_reg);
    reg.loop_gain = uint8_t(gain);
    return write_reg(reg);
}

Result<void, Error> DRV2605L::init(){
    // 1. After powerup, wait at least 250 µs before the DRV2605 device accepts I
    // 2C commands.
    // 2. Assert the EN pin (logic high). The EN pin can be asserted any time during or after the 250-µs wait period.
    auto reg = RegCopy(mode_reg);
    reg.as_bits_mut() = 0x40;
    if(const auto res = write_reg(reg); res.is_err()) return res;

    clock::delay(250us);

    // // rated_voltage_reg = 0X50;
    // WRITE_REG(rated_voltage_reg);

    // // overdrive_clamp_voltage_reg = 0x89;
    // WRITE_REG(overdrive_clamp_voltage_reg);

    // // feedback_control_reg.
    // // mode_reg = 0;
    // WRITE_REG(mode_reg);
	if (const auto res = write_reg(RATED_VOLTAGE_Reg, 0x50); res.is_err()) return res;

	/* Set overdrive voltage */
	if (const auto res = write_reg(OD_CLAMP_Reg, 0x89); res.is_err()) return res;
	
	/* Setup feedback control and control registers */
	if (const auto res = write_reg(FB_CON_Reg, 0xB6); res.is_err()) return res;
	if (const auto res = write_reg(CONTRL1_Reg, 0x13); res.is_err()) return res;
	if (const auto res = write_reg(CONTRL2_Reg, 0xF5); res.is_err()) return res;
	if (const auto res = write_reg(CONTRL3_Reg, 0x80); res.is_err()) return res;
	
	/* Select the LRA Library */
	if (const auto res = write_reg(LIB_SEL_Reg, 0x06); res.is_err()) return res;

	/* Put the DRV2605 device in active mode */
	if (const auto res = write_reg(MODE_Reg, 0x00); res.is_err()) return res;

    return Ok();

}

Result<void, Error> DRV2605L::reset(){
    auto reg = RegCopy(mode_reg);
    reg.dev_reset = 1;
    if(const auto res = write_reg(reg); res.is_err()) return res;
    reg.dev_reset = 0;
    reg.apply();
    return Ok();
}

Result<void, Error> DRV2605L::play(const uint8_t idx){
    if(const auto res = write_reg(RegAddr{0x01}, 0x00); res.is_err()) return res;
    if(const auto res = write_reg(RegAddr{0x04}, idx); res.is_err()) return res;
    if(const auto res = write_reg(RegAddr{0x05}, 0x00); res.is_err()) return res;
    if(const auto res = write_reg(RegAddr{0x0c}, 0x01); res.is_err()) return res;
    return Ok();
}

Result<void, Error> DRV2605L::autocal(){
    uint8_t temp = 0x00;
    uint8_t ACalComp, ACalBEMF, BEMFGain;

	/* Set rate voltage */
	if (const auto res = write_reg(RATED_VOLTAGE_Reg, 0x50); res.is_err()) return res;

	/* Set overdrive voltage */
	if (const auto res = write_reg(OD_CLAMP_Reg, 0x89); res.is_err()) return res;
	
	/* Setup feedback control and control registers */
	if (const auto res = write_reg(FB_CON_Reg, 0xB6); res.is_err()) return res;
	if (const auto res = write_reg(CONTRL1_Reg, 0x93); res.is_err()) return res;
	if (const auto res = write_reg(CONTRL2_Reg, 0xF5); res.is_err()) return res;
	if (const auto res = write_reg(CONTRL3_Reg, 0x80); res.is_err()) return res;
	
	/* Put the DRV2605 device in auto calibration mode */
	if (const auto res = write_reg(MODE_Reg, 0x07); res.is_err()) return res;
    
    if (const auto res = write_reg(CONTRL4_Reg, 0x20); res.is_err()) return res;
	
	/* Begin auto calibration */
    if (const auto res = write_reg(GO_Reg, 0x01); res.is_err()) return res;
	
	/* Poll the GO register until least significant bit is set */
	while (((temp & 0x01) != 0x01)){
		if (const auto res = read_reg(GO_Reg, temp); res.is_err()) return res;
	}
	
	/* Read the result of the auto calibration */
	if (const auto res = read_reg(STATUS_Reg, temp); res.is_err()) return res;

    
	/* Read the compensation result of the auto calibration */
	if (const auto res = read_reg(A_CAL_COMP_Reg, ACalComp); res.is_err()) return res;

	/* Read the Back-EMF result of the auto calibration */
	if (const auto res = read_reg(A_CAL_BEMF_Reg, ACalBEMF); res.is_err()) return res;


	/* Read the feedback control */
	if (const auto res = read_reg(FB_CON_Reg, BEMFGain); res.is_err()) return res;
	
	return Ok();
}