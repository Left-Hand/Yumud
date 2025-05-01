#include "VL6180X.hpp"

#define VL6180X_DEBUG

#ifdef VL6180X_DEBUG
#undef VL6180X_DEBUG
#define VL6180X_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define VL6180X_PANIC(...) PANIC(__VA_ARGS__)
#define VL6180X_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define VL6180X_DEBUG(...)
#define VL6180X_PANIC(...)  PANIC()
#define VL6180X_ASSERT(cond, ...) ASSERT(cond)
#endif

using namespace ymd;
using namespace ymd::drivers;

static constexpr uint16_t ScalerValues[4] = {0, 253, 127, 84};

using Error = VL6180X::Error;
template<typename T = void>
using IResult = Result<T, Error>;


IResult<> VL6180X::verify(){
	static constexpr uint16_t WHO_AM_I_ADDR = 0;
	static constexpr uint8_t KEY = 0xb4;

	uint8_t dummy = 0;
	readReg(WHO_AM_I_ADDR, dummy);
	if (dummy != KEY) return Err(Error::WrongWhoAmI);
	return Ok();
}
// Initialize sensor with settings from ST application note AN4545, section
// "SR03 settings" - "Mandatory : private registers"
void VL6180X::init(){
	verify();
	// Store part-to-part range offset so it can be adjusted if scaling is changed

	readReg(RegAddress::SYSRANGE__PART_TO_PART_RANGE_OFFSET, ptp_offset);

	// #if 1
	// while(true){
	// 	// writeReg(0x207, 0x01);
	// 	uint8_t dummy;
	// 	readReg(RegAddress::SYSTEM__FRESH_OUT_OF_RESET, dummy);
	// 	delay(1);
	// }
	// #endif

	
	if ([&]{
		uint8_t dummy;
		readReg(RegAddress::SYSTEM__FRESH_OUT_OF_RESET, dummy);
		// while(true) DEBUG_PRINTLN(dummy);
		return dummy;
	}() == 1){
		scaling = 1;

		writeReg(0x207, 0x01);
		writeReg(0x208, 0x01);
		writeReg(0x096, 0x00);
		writeReg(0x097, 0xFD); // RANGE_SCALER = 253
		writeReg(0x0E3, 0x01);
		writeReg(0x0E4, 0x03);
		writeReg(0x0E5, 0x02);
		writeReg(0x0E6, 0x01);
		writeReg(0x0E7, 0x03);
		writeReg(0x0F5, 0x02);
		writeReg(0x0D9, 0x05);
		writeReg(0x0DB, 0xCE);
		writeReg(0x0DC, 0x03);
		writeReg(0x0DD, 0xF8);
		writeReg(0x09F, 0x00);
		writeReg(0x0A3, 0x3C);
		writeReg(0x0B7, 0x00);
		writeReg(0x0BB, 0x3C);
		writeReg(0x0B2, 0x09);
		writeReg(0x0CA, 0x09);
		writeReg(0x198, 0x01);
		writeReg(0x1B0, 0x17);
		writeReg(0x1AD, 0x00);
		writeReg(0x0FF, 0x05);
		writeReg(0x100, 0x05);
		writeReg(0x199, 0x05);
		writeReg(0x1A6, 0x1B);
		writeReg(0x1AC, 0x3E);
		writeReg(0x1A7, 0x1F);
		writeReg(0x030, 0x00);

		writeReg(RegAddress::SYSTEM__FRESH_OUT_OF_RESET, 0);
	}else{
		// Sensor has already been initialized, so try to get scaling settings by
		// reading registers.
		uint16_t s ;
		readReg16Bit(RegAddress::RANGE_SCALER, s);

		if      (s == ScalerValues[3]) { scaling = 3; }
		else if (s == ScalerValues[2]) { scaling = 2; }
		else                           { scaling = 1; }

		// Adjust the part-to-part range offset value read earlier to account for
		// existing scaling. If the sensor was already in 2x or 3x scaling mode,
		// precision will be lost calculating the original (1x) offset, but this can
		// be resolved by resetting the sensor and Arduino again.
		ptp_offset *= scaling;
	}
}

// Configure some settings for the sensor's default behavior from AN4545 -
// "Recommended : Public registers" and "Optional: Public registers"
//
// Note that this function does not set up GPIO1 as an interrupt output as
// suggested, though you can do so by calling:
// writeReg(RegAddress::SYSTEM__MODE_GPIO1, 0x10);
void VL6180X::configureDefault()
{
	// "Recommended : Public registers"

	// readout__averaging_sample_period = 48
	writeReg(RegAddress::READOUT__AVERAGING_SAMPLE_PERIOD, 0x30);

	// sysals__analogue_gain_light = 6 (ALS gain = 1 nominal, actually 1.01 according to table "Actual gain values" in datasheet)
	writeReg(RegAddress::SYSALS__ANALOGUE_GAIN, 0x46);

	// sysrange__vhv_repeat_rate = 255 (auto Very High Voltage temperature recalibration after every 255 range measurements)
	writeReg(RegAddress::SYSRANGE__VHV_REPEAT_RATE, 0xFF);

	// sysals__integration_period = 99 (100 ms)
	writeReg16Bit(RegAddress::SYSALS__INTEGRATION_PERIOD, 0x0063);

	// sysrange__vhv_recalibrate = 1 (manually trigger a VHV recalibration)
	writeReg(RegAddress::SYSRANGE__VHV_RECALIBRATE, 0x01);


	// "Optional: Public registers"

	// sysrange__intermeasurement_period = 9 (100 ms)
	writeReg(RegAddress::SYSRANGE__INTERMEASUREMENT_PERIOD, 0x09);

	// sysals__intermeasurement_period = 49 (500 ms)
	writeReg(RegAddress::SYSALS__INTERMEASUREMENT_PERIOD, 0x31);

	// als_int_mode = 4 (ALS new sample ready interrupt); range_int_mode = 4 (range new sample ready interrupt)
	writeReg(RegAddress::SYSTEM__INTERRUPT_CONFIG_GPIO, 0x24);


	// Reset other settings to power-on defaults

	// sysrange__max_convergence_time = 49 (49 ms)
	writeReg(RegAddress::SYSRANGE__MAX_CONVERGENCE_TIME, 0x31);

	// disable interleaved mode
	writeReg(RegAddress::INTERLEAVED_MODE__ENABLE, 0);

	// reset range scaling factor to 1x
	setScaling(1);
}



// Set range scaling factor. The sensor uses 1x scaling by default, giving range
// measurements in units of mm. Increasing the scaling to 2x or 3x makes it give
// raw values in units of 2 mm or 3 mm instead. In other words, a bigger scaling
// factor increases the sensor's potential maximum range but reduces its
// resolution.

// Implemented using ST's VL6180X API as a reference (STSW-IMG003); see
// VL6180x_UpscaleSetScaling() in vl6180x_api.c.
void VL6180X::setScaling(uint8_t new_scaling)
{
	uint8_t const DefaultCrosstalkValidHeight = 20; // default value of SYSRANGE__CROSSTALK_VALID_HEIGHT

	// do nothing if scaling value is invalid
	if (new_scaling < 1 || new_scaling > 3) { return; }

	scaling = new_scaling;
	writeReg16Bit(RegAddress::RANGE_SCALER, ScalerValues[scaling]);

	// apply scaling on part-to-part offset
	writeReg(RegAddress::SYSRANGE__PART_TO_PART_RANGE_OFFSET, ptp_offset / scaling);

	// apply scaling on CrossTalkValidHeight
	writeReg(RegAddress::SYSRANGE__CROSSTALK_VALID_HEIGHT, DefaultCrosstalkValidHeight / scaling);

	// This function does not apply scaling to RANGE_IGNORE_VALID_HEIGHT.

	// enable early convergence estimate only at 1x scaling
	uint8_t rce;
	readReg(RegAddress::SYSRANGE__RANGE_CHECK_ENABLES, rce);
	writeReg(RegAddress::SYSRANGE__RANGE_CHECK_ENABLES, (rce & 0xFE) | (scaling == 1));
}

// Performs a single-shot ranging measurement
uint8_t VL6180X::readRangeSingle()
{
	writeReg(RegAddress::SYSRANGE__START, 0x01);
	return readRangeContinuous();
}

// Performs a single-shot ambient light measurement
uint16_t VL6180X::readAmbientSingle()
{
	writeReg(RegAddress::SYSALS__START, 0x01);
	return readAmbientContinuous();
}

// Starts continuous ranging measurements with the given period in ms
// (10 ms resolution; defaults to 100 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section "Continuous mode limits" in the datasheet
// for details.
void VL6180X::startRangeContinuous(uint16_t period)
{
	const uint8_t period_reg = CLAMP(int16_t(period / 10) - 1, 0, 254);

	writeReg(RegAddress::SYSRANGE__INTERMEASUREMENT_PERIOD, period_reg);
	writeReg(RegAddress::SYSRANGE__START, 0x03);
}

// Starts continuous ambient light measurements with the given period in ms
// (10 ms resolution; defaults to 500 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section "Continuous mode limits" in the datasheet
// for details.
void VL6180X::startAmbientContinuous(uint16_t period){
	const uint8_t raw = CLAMP(int16_t(period / 10) - 1, 0, 254);

	writeReg(RegAddress::SYSALS__INTERMEASUREMENT_PERIOD, raw);
	writeReg(RegAddress::SYSALS__START, 0x03);
}

	// Starts continuous interleaved measurements with the given period in ms
	// (10 ms resolution; defaults to 500 ms if not specified). In this mode, each
	// ambient light measurement is immediately followed by a range measurement.
	//
	// The datasheet recommends using this mode instead of running "range and ALS
	// continuous modes simultaneously (i.e. asynchronously)".
	//
	// The period must be greater than the time it takes to perform both
	// measurements. See section "Continuous mode limits" in the datasheet
	// for details.
void VL6180X::startInterleavedContinuous(uint16_t period){
	const uint8_t raw = CLAMP(int16_t(period / 10) - 1, 0, 254);

	writeReg(RegAddress::INTERLEAVED_MODE__ENABLE, 1);
	writeReg(RegAddress::SYSALS__INTERMEASUREMENT_PERIOD, raw);
	writeReg(RegAddress::SYSALS__START, 0x03);
}


// real_t VL6180X::readLux(uint8_t gain) {
// 	uint8_t reg;

// 	reg = read8(RegAddress::SYSTEM__INTERRUPT_CONFIG_GPIO);
// 	reg &= ~0x38;
// 	reg |= (0x4 << 3); // IRQ on ALS ready
// 	write8(RegAddress::SYSTEM__INTERRUPT_CONFIG_GPIO, reg);

// 	// 100 ms integration period
// 	write8(RegAddress::SYSALS_INTEGRATION_PERIOD_HI, 0);
// 	write8(RegAddress::SYSALS_INTEGRATION_PERIOD_LO, 100);

// 	// analog gain
// 	if (gain > VL6180X_ALS_GAIN_40) {
// 		gain = VL6180X_ALS_GAIN_40;
// 	}
// 	write8(RegAddress::SYSALS_ANALOGUE_GAIN, 0x40 | gain);

// 	// start ALS
// 	write8(RegAddress::SYSALS_START, 0x1);

// 	// Poll until "New Sample Ready threshold event" is set
// 	while (4 != ((read8(RegAddress::RESULT_INTERRUPT_STATUS_GPIO) >> 3) & 0x7))
// 		;

// 	// read lux!
// 	float lux = read16(RegAddress::RESULT_ALS_VAL);

// 	// clear interrupt
// 	write8(RegAddress::SYSTEM_INTERRUPT_CLEAR, 0x07);

// 	lux *= 0.32; // calibrated count/lux
// 	switch (gain) {
// 	case VL6180X_ALS_GAIN_1:
// 		break;
// 	case VL6180X_ALS_GAIN_1_25:
// 		lux /= 1.25;
// 		break;
// 	case VL6180X_ALS_GAIN_1_67:
// 		lux /= 1.67;
// 		break;
// 	case VL6180X_ALS_GAIN_2_5:
// 		lux /= 2.5;
// 		break;
// 	case VL6180X_ALS_GAIN_5:
// 		lux /= 5;
// 		break;
// 	case VL6180X_ALS_GAIN_10:
// 		lux /= 10;
// 		break;
// 	case VL6180X_ALS_GAIN_20:
// 		lux /= 20;
// 		break;
// 	case VL6180X_ALS_GAIN_40:
// 		lux /= 40;
// 		break;
// 	}
// 	lux *= 100;
// 	lux /= 100; // integration time in ms

// 	return lux;
// }

// Stops continuous mode. This will actually start a single measurement of range
// and/or ambient light if continuous mode is not active, so it's a good idea to
// wait a few hundred ms after calling this function to let that complete
// before starting continuous mode again or taking a reading.
void VL6180X::stopContinuous()
{

	writeReg(RegAddress::SYSRANGE__START, 0x01);
	writeReg(RegAddress::SYSALS__START, 0x01);

	writeReg(RegAddress::INTERLEAVED_MODE__ENABLE, 0);
}

// Returns a range reading when continuous mode is activated
// (readRangeSingle() also calls this function after starting a single-shot
// range measurement)
uint8_t VL6180X::readRangeContinuous()
{
	uint16_t millis_start = millis();
	// While computation is not finished
	// only watching if bits 2:0 (mask 0x07) are set to 0b100 (0x04)
	while (([&]{
		uint8_t dummy;
		readReg(RegAddress::RESULT__INTERRUPT_STATUS_GPIO, dummy);
		return dummy;
	}() & 0x07) != 0x04){
		if (io_timeout > 0 && ((uint16_t)millis() - millis_start) > io_timeout)
		{
			did_timeout = true;
			return 255;
		}
	}

	uint8_t range; 
	readReg(RegAddress::RESULT__RANGE_VAL, range);
	writeReg(RegAddress::SYSTEM__INTERRUPT_CLEAR, 0x01);

	return range;
}

// Returns an ambient light reading when continuous mode is activated
// (readAmbientSingle() also calls this function after starting a single-shot
// ambient light measurement)
uint16_t VL6180X::readAmbientContinuous()
{
	uint16_t millis_start = millis();
	// While computation is not finished
	// only watching if bits 5:3 (mask 0x38) are set to 0b100 (0x20)
	while ((
		[&]{
			uint8_t dummy;
			readReg(RegAddress::RESULT__INTERRUPT_STATUS_GPIO, dummy);
			return dummy; 
		}()
		& 0x38) != 0x20)
	{
		if (io_timeout > 0 && (millis() - millis_start) > io_timeout){
			did_timeout = true;
			return 0;
		}
	}

	uint16_t ambient;
	readReg16Bit(RegAddress::RESULT__ALS_VAL, ambient);
	writeReg(RegAddress::SYSTEM__INTERRUPT_CLEAR, 0x02);

	return ambient;
}

// Did a timeout occur in one of the read functions since the last call to
// timeoutOccurred()?
bool VL6180X::timeoutOccurred(){
	bool tmp = did_timeout;
	did_timeout = false;
	return tmp;
}

// Get ranging success/error status code (Use it before using a measurement)
// Return error code; One of possible VL6180X_ERROR_* values
uint8_t VL6180X::readRangeStatus(){
	uint8_t dummy;
	readReg(RegAddress::RESULT__RANGE_STATUS, dummy);
	return dummy >> 4;
}