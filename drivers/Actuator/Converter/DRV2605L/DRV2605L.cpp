#include "DRV2605L.hpp"

#include "core/debug/debug.hpp"

#define DRV2605_DEBUG_EN 0

#if DRV2605_DEBUG_EN

#define DRV2605_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define DRV2605_PANIC(...) PANIC{__VA_ARGS__}
#define DRV2605_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#define READ_REG(reg) read_reg(reg.address, reg).loc().expect();
#define WRITE_REG(reg) write_reg(reg.address, reg).loc().expect();
#else
#define DRV2605_DEBUG(...)
#define DRV2605_PANIC(...)  PANIC_NSRC()
#define DRV2605_ASSERT(cond, ...) ASSERT_NSRC(cond)
#define READ_REG(reg) (void) read_reg(reg.address, reg);
#define WRITE_REG(reg) (void) write_reg(reg.address, reg);
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

void DRV2605L::setFbBrakeFactor(const FbBrakeFactor factor){
    feedback_control_reg.fb_brake_factor = uint8_t(factor);
    WRITE_REG(feedback_control_reg);
}

void DRV2605L::setFbBrakeFactor(const int fractor){
    switch(fractor){
        case 1:setFbBrakeFactor(FbBrakeFactor::_1x);break;
        case 2:setFbBrakeFactor(FbBrakeFactor::_2x);break;
        case 3:setFbBrakeFactor(FbBrakeFactor::_3x);break;
        case 4:setFbBrakeFactor(FbBrakeFactor::_4x);break;
        case 6:setFbBrakeFactor(FbBrakeFactor::_6x);break;
        case 8:setFbBrakeFactor(FbBrakeFactor::_8x);break;
        case 16:setFbBrakeFactor(FbBrakeFactor::_16x);break;
        default:
            DRV2605_PANIC("invalid brake factor");
    }
}

void DRV2605L::setBemfGain(const BemfGain gain){
    feedback_control_reg.bemf_gain = uint8_t(gain);
    WRITE_REG(feedback_control_reg);
}

void DRV2605L::setLoopGain(const LoopGain gain){
    feedback_control_reg.loop_gain = uint8_t(gain);
    WRITE_REG(feedback_control_reg);
}

Result<void, Error> DRV2605L::init(){
    // 1. After powerup, wait at least 250 µs before the DRV2605 device accepts I
    // 2C commands.
    // 2. Assert the EN pin (logic high). The EN pin can be asserted any time during or after the 250-µs wait period.
    
    mode_reg = 0x40;
    WRITE_REG(mode_reg);

    udelay(250);

    // // rated_voltage_reg = 0X50;
    // WRITE_REG(rated_voltage_reg);

    // // overdrive_clamp_voltage_reg = 0x89;
    // WRITE_REG(overdrive_clamp_voltage_reg);

    // // feedback_control_reg.
    // // mode_reg = 0;
    // WRITE_REG(mode_reg);
	if (write_reg(RATED_VOLTAGE_Reg, 0x50).is_err()) return Err(Error(Error::BusFault));

	/* Set overdrive voltage */
	if (write_reg(OD_CLAMP_Reg, 0x89).is_err()) return Err(Error(Error::BusFault));
	
	/* Setup feedback control and control registers */
	if (write_reg(FB_CON_Reg, 0xB6).is_err()) return Err(Error(Error::BusFault));
	if (write_reg(CONTRL1_Reg, 0x13).is_err()) return Err(Error(Error::BusFault));
	if (write_reg(CONTRL2_Reg, 0xF5).is_err()) return Err(Error(Error::BusFault));
	if (write_reg(CONTRL3_Reg, 0x80).is_err()) return Err(Error(Error::BusFault));
	
	/* Select the LRA Library */
	if (write_reg(LIB_SEL_Reg, 0x06).is_err()) return Err(Error(Error::BusFault));

	/* Put the DRV2605 device in active mode */
	if (write_reg(MODE_Reg, 0x00).is_err()) return Err(Error(Error::BusFault));

    return Ok();

}

void DRV2605L::reset(){
    mode_reg.dev_reset = 1;
    WRITE_REG(mode_reg);
    mode_reg.dev_reset = 0;
}

void DRV2605L::play(const uint8_t idx){
    write_reg(0x01, 0x00);
    write_reg(0x04, idx);
    write_reg(0x05, 0x00);
    write_reg(0x0c, 0x01);
}

Result<void, Error> DRV2605L::autocal(){
    uint8_t temp = 0x00;
    uint8_t ACalComp, ACalBEMF, BEMFGain;

	/* Set rate voltage */
	if (write_reg(RATED_VOLTAGE_Reg, 0x50).is_err()) return Err(Error(Error::BusFault));

	/* Set overdrive voltage */
	if (write_reg(OD_CLAMP_Reg, 0x89).is_err()) return Err(Error(Error::BusFault));
	
	/* Setup feedback control and control registers */
	if (write_reg(FB_CON_Reg, 0xB6).is_err()) return Err(Error(Error::BusFault));
	if (write_reg(CONTRL1_Reg, 0x93).is_err()) return Err(Error(Error::BusFault));
	if (write_reg(CONTRL2_Reg, 0xF5).is_err()) return Err(Error(Error::BusFault));
	if (write_reg(CONTRL3_Reg, 0x80).is_err()) return Err(Error(Error::BusFault));
	
	/* Put the DRV2605 device in auto calibration mode */
	if (write_reg(MODE_Reg, 0x07).is_err()) return Err(Error(Error::BusFault));
    
    if (write_reg(CONTRL4_Reg, 0x20).is_err()) return Err(Error(Error::BusFault));
	
	/* Begin auto calibration */
    if (write_reg(GO_Reg, 0x01).is_err()) return Err(Error(Error::BusFault));
	
	/* Poll the GO register until least significant bit is set */
	while (((temp & 0x01) != 0x01)){
		if (read_reg(GO_Reg, temp).is_err()) return Err(Error(Error::BusFault));
	}
	
	/* Read the result of the auto calibration */
	if (read_reg(STATUS_Reg, temp).is_err()) return Err(Error(Error::BusFault));

    
	/* Read the compensation result of the auto calibration */
	if (read_reg(A_CAL_COMP_Reg, ACalComp).is_err()) return Err(Error(Error::BusFault));

	/* Read the Back-EMF result of the auto calibration */
	if (read_reg(A_CAL_BEMF_Reg, ACalBEMF).is_err()) return Err(Error(Error::BusFault));


	/* Read the feedback control */
	if (read_reg(FB_CON_Reg, BEMFGain).is_err()) return Err(Error(Error::BusFault));
	
	return Ok();
}