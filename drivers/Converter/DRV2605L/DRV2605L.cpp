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


static constexpr uint8_t STATUS_Reg          = 0x00;
static constexpr uint8_t MODE_Reg            = 0x01;
static constexpr uint8_t RTP_INPUT_Reg       = 0x02;
static constexpr uint8_t LIB_SEL_Reg         = 0x03;
static constexpr uint8_t WAV_SEQ1_Reg        = 0x04;
static constexpr uint8_t WAV_SEQ2_Reg        = 0x05;
static constexpr uint8_t WAV_SEQ3_Reg        = 0x06;
static constexpr uint8_t WAV_SEQ4_Reg        = 0x07;
static constexpr uint8_t WAV_SEQ5_Reg        = 0x08;
static constexpr uint8_t WAV_SEQ6_Reg        = 0x09;
static constexpr uint8_t WAV_SEQ7_Reg        = 0x0A;
static constexpr uint8_t WAV_SEQ8_Reg        = 0x0B;
static constexpr uint8_t GO_Reg              = 0x0C;
static constexpr uint8_t ODT_OFFSET_Reg      = 0x0D;
static constexpr uint8_t SPT_Reg             = 0x0E;
static constexpr uint8_t SNT_Reg             = 0x0F;
static constexpr uint8_t BRT_Reg             = 0x10;
static constexpr uint8_t ATV_CON_Reg         = 0x11;
static constexpr uint8_t ATV_MIN_IN_Reg      = 0x12;
static constexpr uint8_t ATV_MAX_IN_Reg      = 0x13;
static constexpr uint8_t ATV_MIN_OUT_Reg     = 0x14;
static constexpr uint8_t ATV_MAX_OUT_Reg     = 0x15;
static constexpr uint8_t RATED_VOLTAGE_Reg   = 0x16;
static constexpr uint8_t OD_CLAMP_Reg        = 0x17;
static constexpr uint8_t A_CAL_COMP_Reg      = 0x18;
static constexpr uint8_t A_CAL_BEMF_Reg      = 0x19;
static constexpr uint8_t FB_CON_Reg          = 0x1A;
static constexpr uint8_t CONTRL1_Reg         = 0x1B;
static constexpr uint8_t CONTRL2_Reg         = 0x1C;
static constexpr uint8_t CONTRL3_Reg         = 0x1D;
static constexpr uint8_t CONTRL4_Reg         = 0x1E;
static constexpr uint8_t VBAT_MON_Reg        = 0x21;
static constexpr uint8_t LRA_RESON_Reg       = 0x22;



using namespace ymd;
using namespace ymd::drivers;

using Error = DRV2605L::Error;

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
    if(const auto res = write_reg(0x01, 0x00); res.is_err()) return res;
    if(const auto res = write_reg(0x04, idx); res.is_err()) return res;
    if(const auto res = write_reg(0x05, 0x00); res.is_err()) return res;
    if(const auto res = write_reg(0x0c, 0x01); res.is_err()) return res;
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