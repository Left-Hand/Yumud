#include "vl53l0x.hpp"

#define VL53L0X_REG_IDENTIFICATION_MODEL_ID      		    0x00c0
#define VL53L0X_REG_IDENTIFICATION_REVISION_ID      		0x00c2
#define VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD   		0x0050
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD 		0x0070
#define VL53L0X_REG_SYSRANGE_START                 		    0x0000
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS        		    0x0013
#define VL53L0X_REG_RESULT_RANGE_STATUS            		    0x0014
#define VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS        		0x008a
#define VL53L0X_REG_SYSTEM_RANGE_CONFIG			            0x0009
#define VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV       0x0089
#define VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT                0x0000
#define VL53L0X_REG_SYSRANGE_MODE_START_STOP                0x0001
#define VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK                0x0002
#define VL53L0X_REG_SYSRANGE_MODE_TIMED                     0x0004

#define VL53L0X_DEVICEMODE_SINGLE_RANGING	               ((uint8_t)  0)
#define VL53L0X_DEVICEMODE_CONTINUOUS_RANGING	           ((uint8_t)  1)
#define VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING        ((uint8_t)  3)

#define ESD_2V8

void VL53L0X::init(){
	uint8_t data;
#ifdef ESD_2V8
	data = readByteData(VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV);
	data = (data & 0xFE) | 0x01;
	writeByteData(VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, data);
#endif
	writeByteData(0x88, 0x00);
	writeByteData(0x80, 0x01);
	writeByteData(0xFF, 0x01);
	writeByteData(0x00, 0x00);
	readByteData(0x91);
	writeByteData(0x91, 0x3c);
	writeByteData(0x00, 0x01);
	writeByteData(0xFF, 0x00);
	writeByteData(0x80, 0x00);
}

void VL53L0X::startConv(){	
	if(!continuous){
        writeByteData(0x80, 0x01);
        writeByteData(0xFF, 0x01);
        writeByteData(0x00, 0x00);
        writeByteData(0x91, 0x3c);
        writeByteData(0x00, 0x01);
        writeByteData(0xFF, 0x00);
        writeByteData(0x80, 0x00);
		writeByteData(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_START_STOP);
    }else{
		writeByteData(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK);
	}	
}

bool VL53L0X::isIdle(){
    return !bool(readByteData(VL53L0X_REG_SYSRANGE_START) & 0x01);
}

void VL53L0X::flush(){
	requestData(VL53L0X_REG_RESULT_RANGE_STATUS + 6, (uint8_t *)&result.ambientCount, 6);
}

void VL53L0X::enableHighPrecision(const bool &_highPrec){
    highPrec = _highPrec;
	writeByteData(VL53L0X_REG_SYSTEM_RANGE_CONFIG, highPrec);
}

void VL53L0X::enableContMode(const bool & _continuous){
    continuous = _continuous;
}

void VL53L0X::stop(){
	writeByteData(VL53L0X_REG_SYSRANGE_START, VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT);
	writeByteData(0xFF, 0x01);
	writeByteData(0x00, 0x00);
	writeByteData(0x91, 0x00);
	writeByteData(0x00, 0x01);
	writeByteData(0xFF, 0x00);
}

bool VL53L0X::update(){
    bool conv_done = (isIdle());
    if(conv_done){
        flush();
    }
    return conv_done;
}
uint16_t VL53L0X::getDistance(){
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

uint16_t VL53L0X::getAmbientCount(){
	return result.ambientCount;
}

uint16_t VL53L0X::getSignalCount(){
	return result.signalCount;
}
