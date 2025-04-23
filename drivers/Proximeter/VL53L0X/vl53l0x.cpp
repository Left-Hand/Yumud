#include "vl53l0x.hpp"

using namespace ymd;
using namespace ymd::drivers;


static constexpr uint8_t VL53L0X_REG_IDENTIFICATION_MODEL_ID      		     	= 0x00c0;
static constexpr uint8_t VL53L0X_REG_IDENTIFICATION_REVISION_ID      		 	= 0x00c2;
static constexpr uint8_t VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD   		 	= 0x0050;
static constexpr uint8_t VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD 		 	= 0x0070;
static constexpr uint8_t VL53L0X_REG_SYSRANGE_START								= 0x0000;
static constexpr uint8_t VL53L0X_REG_RESULT_INTERRUPT_STATUS        		    = 0x0013;
static constexpr uint8_t VL53L0X_REG_RESULT_RANGE_STATUS            		    = 0x0014;
static constexpr uint8_t VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS        		 	= 0x008a;
static constexpr uint8_t VL53L0X_REG_SYSTEM_RANGE_CONFIG			            = 0x0009;
static constexpr uint8_t VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV        	= 0x0089;
static constexpr uint8_t VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT                 	= 0x0000;
static constexpr uint8_t VL53L0X_REG_SYSRANGE_MODE_START_STOP                 	= 0x0001;
static constexpr uint8_t VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK                 	= 0x0002;
static constexpr uint8_t VL53L0X_REG_SYSRANGE_MODE_TIMED                      	= 0x0004;

#define VL53L0X_DEVICEMODE_SINGLE_RANGING	               ((uint8_t)  0)
#define VL53L0X_DEVICEMODE_CONTINUOUS_RANGING	           ((uint8_t)  1)
#define VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING        ((uint8_t)  3)

#define ESD_2V8

void VL53L0X::init(){
	uint8_t data;
#ifdef ESD_2V8
	data = read_byte_data(VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV);
	data = (data & 0xFE) | 0x01;
	write_byte_data(VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, data);
#endif
	write_byte_data(0x88, 0x00);
	write_byte_data(0x80, 0x01);
	write_byte_data(0xFF, 0x01);
	write_byte_data(0x00, 0x00);
	read_byte_data(0x91);
	write_byte_data(0x91, 0x3c);
	write_byte_data(0x00, 0x01);
	write_byte_data(0xFF, 0x00);
	write_byte_data(0x80, 0x00);
}

void VL53L0X::startConv(){	
	if(!continuous){
        write_byte_data(0x80, 0x01);
        write_byte_data(0xFF, 0x01);
        write_byte_data(0x00, 0x00);
        write_byte_data(0x91, 0x3c);
        write_byte_data(0x00, 0x01);
        write_byte_data(0xFF, 0x00);
        write_byte_data(0x80, 0x00);
		write_byte_data(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_START_STOP);
    }else{
		write_byte_data(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK);
	}	
}

bool VL53L0X::busy(){
    return bool(read_byte_data(VL53L0X_REG_SYSRANGE_START) & 0x01);
}

void VL53L0X::flush(){
	read_burst(VL53L0X_REG_RESULT_RANGE_STATUS + 6, &result.ambientCount, 3);
}

void VL53L0X::enable_high_precision(const bool _highPrec){
    highPrec = _highPrec;
	write_byte_data(VL53L0X_REG_SYSTEM_RANGE_CONFIG, highPrec);
}

void VL53L0X::enable_cont_mode(const bool _continuous){
    continuous = _continuous;
}

void VL53L0X::stop(){
	write_byte_data(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT);
	write_byte_data(0xFF, 0x01);
	write_byte_data(0x00, 0x00);
	write_byte_data(0x91, 0x00);
	write_byte_data(0x00, 0x01);
	write_byte_data(0xFF, 0x00);
}

void VL53L0X::update(){
    bool onbusy = (busy());
    if(onbusy){
        flush();
    }
}


uint16_t VL53L0X::get_distance_mm(){
    uint16_t ret;
    if(result.distance <= 20 || result.distance  > 2400){
        ret = last_result.distance;
    }else{
        ret = result.distance;
        last_result.distance = result.distance;
    }
    // ret = result.distance;

	if(highPrec) return ret / 4;
	else return ret;
}

uint16_t VL53L0X::get_ambient_count(){
	return result.ambientCount;
}

uint16_t VL53L0X::get_signal_count(){
	return result.signalCount;
}
