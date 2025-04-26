#define VL53L0X_ESD_2V8


#include "vl53l0x.hpp"


using namespace ymd;
using namespace ymd::drivers;


static constexpr uint8_t VL53L0X_REG_IDENTIFICATION_MODEL_ID      		     	= 0xc0;
static constexpr uint8_t VL53L0X_REG_IDENTIFICATION_REVISION_ID      		 	= 0xc2;
static constexpr uint8_t VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD   		 	= 0x50;
static constexpr uint8_t VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD 		 	= 0x70;
static constexpr uint8_t VL53L0X_REG_SYSRANGE_START								= 0x00;
static constexpr uint8_t VL53L0X_REG_RESULT_INTERRUPT_STATUS        		    = 0x13;
static constexpr uint8_t VL53L0X_REG_RESULT_RANGE_STATUS            		    = 0x14;
static constexpr uint8_t VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS        		 	= 0x8a;
static constexpr uint8_t VL53L0X_REG_SYSTEM_RANGE_CONFIG			            = 0x09;
static constexpr uint8_t VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV        	= 0x89;
static constexpr uint8_t VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT                 	= 0x00;
static constexpr uint8_t VL53L0X_REG_SYSRANGE_MODE_START_STOP                 	= 0x01;
static constexpr uint8_t VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK                 	= 0x02;
static constexpr uint8_t VL53L0X_REG_SYSRANGE_MODE_TIMED                      	= 0x04;

static constexpr uint8_t VL53L0X_DEVICEMODE_SINGLE_RANGING	                = ((uint8_t)  0);
static constexpr uint8_t VL53L0X_DEVICEMODE_CONTINUOUS_RANGING	            = ((uint8_t)  1);
static constexpr uint8_t VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING        = ((uint8_t)  3);

using Error = VL53L0X::Error;


Result<void, Error> VL53L0X::init(){
	uint8_t data;
#ifdef VL53L0X_ESD_2V8
	if(const auto res = 
		read_byte_data(VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, data);
		res.is_err()) return Err(res.unwrap_err());
	data = (data & 0xFE) | 0x01;
	if(const auto res = 
		write_byte_data(VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, data);
		res.is_err()) return Err(res.unwrap_err());
#endif
	return write_byte_data(0x88, 0x00)
	| write_byte_data(0x80, 0x01)
	| write_byte_data(0xFF, 0x01)
	| write_byte_data(0x00, 0x00)
	| read_byte_data(0x91, data)
	| write_byte_data(0x91, 0x3c)
	| write_byte_data(0x00, 0x01)
	| write_byte_data(0xFF, 0x00)
	| write_byte_data(0x80, 0x00);
}

Result<void, Error> VL53L0X::start_conv(){	
	if(!continuous_){
			return write_byte_data(0x80, 0x01)
				| write_byte_data(0xFF, 0x01)
				| write_byte_data(0x00, 0x00)
				| write_byte_data(0x91, 0x3c)
				| write_byte_data(0x00, 0x01)
				| write_byte_data(0xFF, 0x00)
				| write_byte_data(0x80, 0x00)
				| write_byte_data(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_START_STOP);
    }else{
		return write_byte_data(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK);
	}	
}

Result<bool, Error> VL53L0X::is_busy(){
	uint8_t data;
    if(const auto err = read_byte_data(VL53L0X_REG_SYSRANGE_START, data); err.is_err())
		return Err(Error(err.unwrap_err()));
	return Ok(data & 0x01);
}

Result<void, Error> VL53L0X::flush(){
	return read_burst(VL53L0X_REG_RESULT_RANGE_STATUS + 6, &result.ambientCount, 3);
}

Result<void, Error> VL53L0X::enable_high_precision(const bool _highPrec){
    highPrec_ = _highPrec;
	return write_byte_data(VL53L0X_REG_SYSTEM_RANGE_CONFIG, highPrec_);
}

Result<void, Error> VL53L0X::enable_cont_mode(const bool _continuous){
    continuous_ = _continuous;
	return Ok();
}

Result<void, Error> VL53L0X::stop(){
	return 
		write_byte_data(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT)
		| write_byte_data(0xFF, 0x01)
		| write_byte_data(0x00, 0x00)
		| write_byte_data(0x91, 0x00)
		| write_byte_data(0x00, 0x01)
		| write_byte_data(0xFF, 0x00)
	;
}

Result<void, Error> VL53L0X::update(){
    if(const auto res = is_busy(); res.is_ok()){
		if(res.unwrap()){
			this->flush();
		}
		return Ok();
	}else{
		return Err(res.unwrap_err());
	}
}


Result<uint16_t, Error> VL53L0X::get_distance_mm(){
    uint16_t ret;
    if(result.distance <= 20 || result.distance  > 2400){
        ret = last_result.distance;
    }else{
        ret = result.distance;
        last_result.distance = result.distance;
    }
    // ret = result.distance;

	if(highPrec_) return Ok(ret / 4);
	else return Ok(ret);
}

Result<uint16_t, Error> VL53L0X::get_ambient_count(){
	return Ok(result.ambientCount);
}

Result<uint16_t, Error> VL53L0X::get_signal_count(){
	return Ok(result.signalCount);
}
