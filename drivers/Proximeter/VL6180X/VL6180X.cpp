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



using Error = VL6180X::Error;
template<typename T = void>
using IResult = Result<T, Error>;


static constexpr uint16_t ScalerValues[4] = {0, 253, 127, 84};

using Pair = std::pair<uint16_t, uint8_t>;
static constexpr Pair INIT_MAP[] = {
	Pair{0x207, 0x01},
	Pair{0x208, 0x01},
	Pair{0x096, 0x00},
	Pair{0x097, 0xFD},
	Pair{0x0E3, 0x01},
	Pair{0x0E4, 0x03},
	Pair{0x0E5, 0x02},
	Pair{0x0E6, 0x01},
	Pair{0x0E7, 0x03},
	Pair{0x0F5, 0x02},
	Pair{0x0D9, 0x05},
	Pair{0x0DB, 0xCE},
	Pair{0x0DC, 0x03},
	Pair{0x0DD, 0xF8},
	Pair{0x09F, 0x00},
	Pair{0x0A3, 0x3C},
	Pair{0x0B7, 0x00},
	Pair{0x0BB, 0x3C},
	Pair{0x0B2, 0x09},
	Pair{0x0CA, 0x09},
	Pair{0x198, 0x01},
	Pair{0x1B0, 0x17},
	Pair{0x1AD, 0x00},
	Pair{0x0FF, 0x05},
	Pair{0x100, 0x05},
	Pair{0x199, 0x05},
	Pair{0x1A6, 0x1B},
	Pair{0x1AC, 0x3E},
	Pair{0x1A7, 0x1F},
	Pair{0x030, 0x00},
};

IResult<> VL6180X::validate(){
	static constexpr uint16_t WHO_AM_I_ADDR = 0;
	static constexpr uint8_t KEY = 0xb4;

	uint8_t dummy = 0;
	if(const auto res = read_reg(WHO_AM_I_ADDR, dummy);
		res.is_err()) return res;
	if (dummy != KEY) return Err(Error::WrongWhoAmI);
	return Ok();
}
// Initialize sensor with settings from ST application note AN4545, section
// "SR03 settings" - "Mandatory : private registers"
IResult<> VL6180X::init(){
	if(const auto res = validate();
		res.is_err()) return res;

	if(const auto res = read_reg(RegAddress::SYSRANGE__PART_TO_PART_RANGE_OFFSET, ptp_offset);
		res.is_err()) return res;
	
	if (
		({
			uint8_t dummy;
			if(const auto res = read_reg(RegAddress::SYSTEM__FRESH_OUT_OF_RESET, dummy);
				res.is_err()) return res;
			dummy;
		}) == 1)
	{
		scaling = 1;

		for(const auto &[addr, val] : INIT_MAP){
			if(const auto res = write_reg(addr, val);
				res.is_err()) return res;
		}

		if(const auto res = write_reg(RegAddress::SYSTEM__FRESH_OUT_OF_RESET, 0);
			res.is_err()) return res;
	}else{
		// Sensor has already been initialized, so try to get scaling settings by
		// reading registers.
		uint16_t s ;
		if(const auto res = read_reg16_bit(RegAddress::RANGE_SCALER, s);
			res.is_err()) return res;

		if      (s == ScalerValues[3]) { scaling = 3; }
		else if (s == ScalerValues[2]) { scaling = 2; }
		else                           { scaling = 1; }

		// Adjust the part-to-part range offset value read earlier to account for
		// existing scaling. If the sensor was already in 2x or 3x scaling mode,
		// precision will be lost calculating the original (1x) offset, but this can
		// be resolved by resetting the sensor and Arduino again.
		ptp_offset *= scaling;
	}

	return Ok();
}

// Configure some settings for the sensor's default behavior from AN4545 -
// "Recommended : Public registers" and "Optional: Public registers"
//
// Note that this function does not set up GPIO1 as an interrupt output as
// suggested, though you can do so by calling:
// write_reg(RegAddress::SYSTEM__MODE_GPIO1, 0x10);
IResult<> VL6180X::configure_default(){
	// "Recommended : Public registers"

	// readout__averaging_sample_period = 48
	if(const auto res = write_reg(RegAddress::READOUT__AVERAGING_SAMPLE_PERIOD, 0x30);
		res.is_err()) return res;

	// sysals__analogue_gain_light = 6 (ALS gain = 1 nominal, actually 1.01 according to table "Actual gain values" in datasheet)
	if(const auto res = write_reg(RegAddress::SYSALS__ANALOGUE_GAIN, 0x46);
		res.is_err()) return res;

	// sysrange__vhv_repeat_rate = 255 (auto Very High Voltage temperature recalibration after every 255 range measurements)
	if(const auto res = write_reg(RegAddress::SYSRANGE__VHV_REPEAT_RATE, 0xFF);
		res.is_err()) return res;

	// sysals__integration_period = 99 (100 ms)
	if(const auto res = write_reg16_bit(RegAddress::SYSALS__INTEGRATION_PERIOD, 99);
		res.is_err()) return res;

	// sysrange__vhv_recalibrate = 1 (manually trigger a VHV recalibration)
	if(const auto res = write_reg(RegAddress::SYSRANGE__VHV_RECALIBRATE, 0x01);
		res.is_err()) return res;


	// "Optional: Public registers"

	// sysrange__intermeasurement_period = 9 (100 ms)
	if(const auto res = write_reg(RegAddress::SYSRANGE__INTERMEASUREMENT_PERIOD, 0x09);
		res.is_err()) return res;

	// sysals__intermeasurement_period = 49 (500 ms)
	if(const auto res = write_reg(RegAddress::SYSALS__INTERMEASUREMENT_PERIOD, 0x31);
		res.is_err()) return res;

	// als_int_mode = 4 (ALS new sample ready interrupt); range_int_mode = 4 (range new sample ready interrupt)
	if(const auto res = write_reg(RegAddress::SYSTEM__INTERRUPT_CONFIG_GPIO, 0x24);
		res.is_err()) return res;


	// Reset other settings to power-on defaults

	// sysrange__max_convergence_time = 49 (49 ms)
	if(const auto res = write_reg(RegAddress::SYSRANGE__MAX_CONVERGENCE_TIME, 0x31);
		res.is_err()) return res;

	// disable interleaved mode
	if(const auto res = write_reg(RegAddress::INTERLEAVED_MODE__ENABLE, 0);
		res.is_err()) return res;

	// reset range scaling factor to 1x
	return set_scaling(1);
}



// Set range scaling factor. The sensor uses 1x scaling by default, giving range
// measurements in units of mm. Increasing the scaling to 2x or 3x makes it give
// raw values in units of 2 mm or 3 mm instead. In other words, a bigger scaling
// factor increases the sensor's potential maximum range but reduces its
// resolution.

// Implemented using ST's VL6180X API as a reference (STSW-IMG003); see
// VL6180x_UpscaleSetScaling() in vl6180x_api.c.
IResult<> VL6180X::set_scaling(uint8_t new_scaling){
	uint8_t const DefaultCrosstalkValidHeight = 20; // default value of SYSRANGE__CROSSTALK_VALID_HEIGHT

	// do nothing if scaling value is invalid
	if (new_scaling < 1 || new_scaling > 3) { return Err(Error::InvalidScaling); }

	scaling = new_scaling;
	if(const auto res =write_reg16_bit(RegAddress::RANGE_SCALER, ScalerValues[scaling]);
		res.is_err()) return res;

	// apply scaling on part-to-part offset
	if(const auto res = write_reg(RegAddress::SYSRANGE__PART_TO_PART_RANGE_OFFSET, ptp_offset / scaling);
		res.is_err()) return res;

	// apply scaling on CrossTalkValidHeight
	if(const auto res = write_reg(RegAddress::SYSRANGE__CROSSTALK_VALID_HEIGHT, DefaultCrosstalkValidHeight / scaling);
		res.is_err()) return res;

	// This function does not apply scaling to RANGE_IGNORE_VALID_HEIGHT.

	// enable early convergence estimate only at 1x scaling
	uint8_t rce;
	if(const auto res = read_reg(RegAddress::SYSRANGE__RANGE_CHECK_ENABLES, rce);
		res.is_err()) return res;
	if(const auto res = write_reg(RegAddress::SYSRANGE__RANGE_CHECK_ENABLES, 
		(rce & 0xFE) | (scaling == 1));
		res.is_err()) return res;

	return Ok();
}


// Starts continuous ranging measurements with the given period in ms
// (10 ms resolution; defaults to 100 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section "Continuous mode limits" in the datasheet
// for details.
IResult<> VL6180X::start_range_continuous(uint16_t period)
{
	const uint8_t period_reg = CLAMP(int16_t(period / 10) - 1, 0, 254);

	if(const auto res = write_reg(RegAddress::SYSRANGE__INTERMEASUREMENT_PERIOD, period_reg);
		res.is_err()) return res;
	if(const auto res = write_reg(RegAddress::SYSRANGE__START, 0x03);
		res.is_err()) return res;

	return Ok();
}

// Starts continuous ambient light measurements with the given period in ms
// (10 ms resolution; defaults to 500 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section "Continuous mode limits" in the datasheet
// for details.
IResult<> VL6180X::start_ambient_continuous(uint16_t period){
	const uint8_t raw = CLAMP(int16_t(period / 10) - 1, 0, 254);

	if(const auto res = write_reg(RegAddress::SYSALS__INTERMEASUREMENT_PERIOD, raw);
		res.is_err()) return res;
	if(const auto res = write_reg(RegAddress::SYSALS__START, 0x03);
		res.is_err()) return res;

	return Ok();
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
IResult<> VL6180X::start_interleaved_continuous(uint16_t period){
	const uint8_t raw = CLAMP(int16_t(period / 10) - 1, 0, 254);

	if(const auto res = write_reg(RegAddress::INTERLEAVED_MODE__ENABLE, 1);
		res.is_err()) return res;
	if(const auto res = write_reg(RegAddress::SYSALS__INTERMEASUREMENT_PERIOD, raw);
		res.is_err()) return res;
	if(const auto res = write_reg(RegAddress::SYSALS__START, 0x03);
		res.is_err()) return res;

	return Ok();
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
IResult<> VL6180X::stop_continuous(){
	if(const auto res = write_reg(RegAddress::SYSRANGE__START, 0x01);
		res.is_err()) return res;
	if(const auto res = write_reg(RegAddress::SYSALS__START, 0x01);
		res.is_err()) return res;
	if(const auto res = write_reg(RegAddress::INTERLEAVED_MODE__ENABLE, 0);
		res.is_err()) return res;

	return Ok();
}

// Returns a range reading when continuous mode is activated
// (readRangeSingle() also calls this function after starting a single-shot
// range measurement)
IResult<uint8_t> VL6180X::read_range(){
	// While computation is not finished
	// only watching if bits 2:0 (mask 0x07) are set to 0b100 (0x04)
	if((({
		uint8_t dummy;
		if(const auto res = read_reg(RegAddress::RESULT__INTERRUPT_STATUS_GPIO, dummy);
			res.is_err()) return Err(res.unwrap_err());
		dummy;
	}) & 0x07) != 0x04) return Err(Error::RangeDataNotReady);

	uint8_t range; 
	if(const auto res = read_reg(RegAddress::RESULT__RANGE_VAL, range);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_reg(RegAddress::SYSTEM__INTERRUPT_CLEAR, 0x01);
		res.is_err()) return Err(res.unwrap_err());

	return Ok(range);
}

// Returns an ambient light reading when continuous mode is activated
// (readAmbientSingle() also calls this function after starting a single-shot
// ambient light measurement)
IResult<uint16_t> VL6180X::read_ambient(){
	// While computation is not finished
	// only watching if bits 5:3 (mask 0x38) are set to 0b100 (0x20)
	if(
		(({
			uint8_t dummy;
			if(const auto res = read_reg(RegAddress::RESULT__INTERRUPT_STATUS_GPIO, dummy);
				res.is_err()) return Err(res.unwrap_err());
			dummy; 
		}) & 0x38) != 0x20) return Err(Error::AmbientDataNotReady);

	uint16_t ambient;
	if(const auto res = read_reg16_bit(RegAddress::RESULT__ALS_VAL, ambient);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_reg(RegAddress::SYSTEM__INTERRUPT_CLEAR, 0x02);
		res.is_err()) return Err(res.unwrap_err());

	return Ok(ambient);
}

// Get ranging success/error status code (Use it before using a measurement)
// Return error code; One of possible VL6180X_ERROR_* values
IResult<uint8_t> VL6180X::read_range_status(){
	uint8_t dummy;
	if(const auto res = read_reg(RegAddress::RESULT__RANGE_STATUS, dummy);
		res.is_err()) return Err(res.unwrap_err());
	return Ok(dummy >> 4);
}

namespace ymd{

OutputStream & operator<<(OutputStream & os, const drivers::VL6180X::Error & err){
	using Kind = drivers::VL6180X::Error_Kind;
	if(err.is<Kind>()) return os << err.as<Kind>().unwrap();
	else return os << err.as<hal::HalError>().unwrap();
}
OutputStream & operator<<(OutputStream & os, const drivers::VL6180X::Error_Kind & err_kind){
	using Kind = drivers::VL6180X::Error_Kind;
	switch(err_kind){
		case Kind::WrongWhoAmI: return os << "WrongWhoAmI";
		case Kind::InvalidScaling: return os << "InvalidScaling";
		case Kind::RangeDataNotReady: return os << "RangeDataNotReady";
		case Kind::AmbientDataNotReady: return os << "AmbientDataNotReady";
		default: __builtin_unreachable();
	}
}
}