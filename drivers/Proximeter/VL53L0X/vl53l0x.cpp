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

static constexpr uint8_t VL53L0X_DEVICEMODE_SINGLE_RANGING	                = 0;
static constexpr uint8_t VL53L0X_DEVICEMODE_CONTINUOUS_RANGING	            = 1;
static constexpr uint8_t VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING        = 3;

using Error = VL53L0X::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> VL53L0X::read_byte_data(const uint8_t reg, uint8_t & data){
	const auto res = i2c_drv_.read_reg(reg, data);
	if(res.is_err()) return Err(res.unwrap_err());
	return Ok();
}

IResult<> VL53L0X::read_burst(const uint8_t reg, const std::span<uint16_t> pbuf){
	const auto res = i2c_drv_.read_burst(reg, pbuf, std::endian::big);
	if(res.is_err()) return Err(res.unwrap_err());
	return Ok();
}

IResult<> VL53L0X::write_byte_data(const uint8_t reg, const uint8_t byte){
	const auto res = i2c_drv_.write_reg(reg, byte);
	if(res.is_err()) return Err(res.unwrap_err());
	return Ok();
}


IResult<> VL53L0X::init(){
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
	if(const auto res = write_byte_data(0x88, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0x80, 0x01);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0xFF, 0x01);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0x00, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = read_byte_data(0x91, data);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0x91, 0x3c);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0x00, 0x01);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0xFF, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0x80, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	return Ok();
}

IResult<> VL53L0X::start_conv(){	
	if(continuous_en_){
		return write_byte_data(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK);
	}

	if(const auto res = write_byte_data(0x80, 0x01);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0xFF, 0x01);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0x00, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0x91, 0x3c);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0x00, 0x01);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0xFF, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0x80, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_START_STOP);
		res.is_err()) return Err(res.unwrap_err());
	return Ok();

}

IResult<bool> VL53L0X::is_busy(){
	uint8_t val;
    if(const auto err = read_byte_data(VL53L0X_REG_SYSRANGE_START, val); err.is_err())
		return Err(err.unwrap_err());
	return Ok(val & 0x01);
}

IResult<> VL53L0X::flush(){
	return read_burst(
		VL53L0X_REG_RESULT_RANGE_STATUS + 6, std::span(&result.ambient_count, 3));
}

IResult<> VL53L0X::enable_high_precision(const Enable en){
    high_prec_en_ = en == EN;
	return write_byte_data(VL53L0X_REG_SYSTEM_RANGE_CONFIG, high_prec_en_);
}

IResult<> VL53L0X::enable_cont_mode(const Enable en){
    continuous_en_ = en == EN;
	return Ok();
}

IResult<> VL53L0X::stop(){
	if(const auto res = write_byte_data(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0xFF, 0x01);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0x00, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0x91, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0x00, 0x01);
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = write_byte_data(0xFF, 0x00);
		res.is_err()) return Err(res.unwrap_err());
	return Ok();
}

IResult<> VL53L0X::update(){
	const bool device_is_busy = ({
		const auto res = is_busy();
		if(res.is_err()) return Err(res.unwrap_err());
		res.unwrap();
	});

	if(device_is_busy == false){
		return this->flush();
	}
	return Ok();
}


IResult<uint16_t> VL53L0X::read_distance_mm(){
    uint16_t ret;
    if(result.distance_mm <= 20 || result.distance_mm > 2400){
        ret = last_result.distance_mm;
    }else{
        ret = result.distance_mm;
        last_result.distance_mm = result.distance_mm;
    }

	if(high_prec_en_) return Ok(ret / 4);
	else return Ok(ret);
}

IResult<uint16_t> VL53L0X::read_ambient_count(){
	return Ok(result.ambient_count);
}

IResult<uint16_t> VL53L0X::read_signal_count(){
	return Ok(result.signal_count);
}
