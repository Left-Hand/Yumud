#include "paj7620.hpp"

#include <utility>

using namespace ymd::drivers;

#ifdef PAJ7620_DEBUG
#undef PAJ7620_DEBUG
#define PAJ7620_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define PAJ7620_PANIC(...) PANIC(__VA_ARGS__)
#define PAJ7620_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define PAJ7620_DEBUG(...)
#define PAJ7620_PANIC(...)  PANIC()
#define PAJ7620_ASSERT(cond, ...) ASSERT(cond)
#endif

#define WRITE_REG(reg) this->writeReg(reg.address, reg);
#define READ_REG(reg) this->readReg(reg.address, reg);




/* Register bank select R/W*/
#define GESTURE_REG_BANK_SEL (0xEF)

/* Bank 0 register, setting the address (R-Read, W-Write)
#define GESTURE_ADDR_PARTID_L (0x00)
#define GESTURE_ADDR_PARTID_H (0x01)
#define GESTURE_ADDR_VERSIONID (0x02) */
#define GESTURE_ADDR_SUSPEND_ENL (0x03) /* R */
#define GESTURE_ADDR_GEST_DETECT_INT1 (0x41) /* R/W */
#define GESTURE_ADDR_GEST_DETECT_INT2 (0x42) /* R/W */
#define GESTURE_ADDR_GEST_DETECT_INTFLAG1 (0x43) /* R */
#define GESTURE_ADDR_GEST_DETECT_INTFLAG2 (0x44) /* R */
#define GESTURE_ADDR_STATE_INDIC (0x45) /* R */
#define GESTURE_ADDR_PS_HIGH_THRESH (0x69) /* R/W */
#define GESTURE_ADDR_PS_LOW_THRESH (0x6A) /* R/W */
#define GESTURE_ADDR_PS_APPROACH_STATE (0x6B) /* R */
#define GESTURE_ADDR_PS_DATA (0x6C) /* R */

#define GESTURE_BANK0 0x00 /* Define Value for Bank 0 */

/* Bank 1 Registers, setting the address */
#define GESTURE_ADDR_IDLE_S1_STEP1 (0x67) /* R/W */
#define GESTURE_ADDR_IDLE_S1_STEP2 (0X68) /* R/W */
#define GESTURE_ADDR_IDLE_S2_STEP1 (0x69) /* R/W */
#define GESTURE_ADDR_IDLE_S2_STEP2 (0x6A) /* R/W */
#define GESTURE_ADDR_OP_TO_S1_STEP1 (0x6B) /* R/W */
#define GESTURE_ADDR_OP_TO_S1_STEP2 (0x6C) /* R/W */
#define GESTURE_ADDR_OP_TO_S2_STEP1 (0x6D) /* R/W */
#define GESTURE_ADDR_OP_TO_S2_STEP2 (0x6E) /* R/W */
#define GESTURE_ADDR_OP_ENABLE (0x72) /* R/W */
#define GESTURE_ADDR_PS_GAIN (0x44) /* R/W */

#define GESTURE_BANK1 0x01 /* Define Value for Bank 1 */


#define GESTURE_ENABLE 0x01 /* Enable address operations */
#define GESTURE_DISABLE 0x00 /* Disable */

#define GESTURE_I2C_SUSPEND 0x00 /* Address suspend CMD */
#define GESTURE_I2C_WAKEUP 0x01

#define GEST_REACT_T 400 /* Gesture reaction time has to be less than gesture entry time for forward/backwards movement */
#define GEST_ENTR_T 600 /* Both can be adjusted to needs */
#define GEST_QUIT_T 1000


#define GESTURE_Value(Value, maskbit) (Value << maskbit)

#define GestureRight GESTURE_Value(1,0)
#define GestureLeft GESTURE_Value(1,1)
#define GestureDown GESTURE_Value(1,2)
#define GestureUp GESTURE_Value(1,3)
#define GestureForward GESTURE_Value(1,4)
#define GestureBackwards GESTURE_Value(1,5)
#define GestureCW GESTURE_Value(1,6)
#define GestureACW GESTURE_Value(1,7)

#define PAJ7620_REGITER_BANK_SEL 0xEF

scexpr uint8_t GES_RIGHT_FLAG				 = (1<<0);
scexpr uint8_t GES_LEFT_FLAG				 = (1<<1);
scexpr uint8_t GES_UP_FLAG					 = (1<<2);
scexpr uint8_t GES_DOWN_FLAG				 = (1<<3);
scexpr uint8_t GES_FORWARD_FLAG				 = (1<<4);
scexpr uint8_t GES_BACKWARD_FLAG			 = (1<<5);
scexpr uint8_t GES_CLOCKWISE_FLAG			 = (1<<6);
scexpr uint8_t GES_COUNT_CLOCKWISE_FLAG		 = (1<<7);
scexpr uint8_t GES_WAVE_FLAG				 = (1<<0);


void PAJ7620::GESTURE_Actions(void){
    //TODO

	// uint8_t data = 0, data1 = 0, error;

	// if(DBG != step){
	// 	DBG = step;
	// }
	// switch(step)
	// {
	// case 0:
	// 	gestureInit();
	// 	step++;
	// 	break;
	// case 1:
	// 	if(counter == 0){
	// 		error = readReg(0x43, 1, &data); /* Read Bank 0 for gestures */

	// 			switch(data){ /* When will gestures be detected, the "data" will change */
	// 			case GestureRight:
	// 				if(counter == 0){
	// 					readReg(0x43, 1, &data);
	// 					if(data == GestureForward){
	// 						strcpy((char*)ref, "\r\n Forward Gesture");
	// 					}else if(data == GestureBackwards){
	// 						strcpy((char*)ref, "\r\n Backwards Gesture");
	// 					}else{
	// 						strcpy((char*)ref, "\r\n Gesture Right");
	// 					}
	// 				}
	// 				break;

	// 			case GestureLeft:
	// 				if(counter == 0){
	// 					readReg(0x43, 1, &data);
	// 					if(data == GestureForward){
	// 						strcpy((char*)ref, "\r\n Forward Gesture");
	// 					}else if(data == GestureBackwards){
	// 						strcpy((char*)ref, "\r\n Backwards Gesture");
	// 					}else{
	// 						strcpy((char*)ref, "\r\n Gesture Left");
	// 					}
	// 				}
	// 					break;

	// 			case GestureUp:
	// 				if(counter == 0){
	// 					readReg(0x43, 1, &data);
	// 					if(data == GestureForward){
	// 						strcpy((char*)ref, "\n\r Forward Gesture");
	// 					}else if(data == GestureBackwards){
	// 						strcpy((char*)ref, "\r\n Backwards Gesture");
	// 					}else{
	// 						strcpy((char*)ref, "\r\n Upwards Gesture");
	// 					}
	// 					break;
	// 				}
	// 			case GestureDown:
	// 				if(counter == 0){
	// 					readReg(0x43, 1, &data);
	// 					if(data == GestureForward){
	// 						strcpy((char*)ref, "\r\n Forward Gesture");
	// 					}else if(data == GestureBackwards){
	// 						strcpy((char*)ref, "\n\r Backwards Gesture");
	// 					}else{
	// 						strcpy((char*)ref, "\r\n Downwards Gesture");
	// 					}
	// 				}
	// 					break;

	// 			case GestureCW:
	// 					strcpy((char*)ref, "\r\n Clockwise Gesture");
	// 					break;

	// 			case GestureACW:
	// 					strcpy((char*)ref, "\r\n Anti-clockwise Gesture");
	// 					break;

	// 			case GestureForward:
	// 					strcpy((char*)ref, "\r\n Forward Gesture");
	// 					break;

	// 			case GestureBackwards:
	// 					strcpy((char*)ref, "\r\n Backwards Gesture");
	// 					break;
	// 			}
	// 		}
	// }
};


void PAJ7620::readReg(uint8_t addr, uint8_t qty, uint8_t * data){
	i2c_drv_.readMulti<uint8_t>(addr, data, qty);
};

void PAJ7620::writeReg(uint8_t addr, uint8_t cmd){
	i2c_drv_.writeReg<uint8_t>(addr, cmd);
};

void PAJ7620::selectBank(uint8_t bank) {
	switch(bank){
		case 9:
			writeReg(PAJ7620_REGITER_BANK_SEL, 0);
			break;
		case 1:
			writeReg(PAJ7620_REGITER_BANK_SEL, 1);
			break;
		default:
			break;
	}
}

void PAJ7620::update(){

}

uint8_t PAJ7620::detect(){
	uint8_t data;
	readReg(0x43, 1, &data);				// Read Bank_0_Reg_0x43/0x44 for gesture result.
	return data;

}
bool PAJ7620::verify(){
	uint8_t data0 = 0, data1 = 0;
	
	delay(1);
	selectBank(0);
	selectBank(0);

	readReg(0,1, &data0);
	readReg(1,1, &data1);

	return (data0 == 0x20) and (data1 == 0x76);
}

scexpr auto initRegisterArray = std::to_array<std::pair<uint8_t, uint8_t>>({
	{0xEF,0x00},
	{0x32,0x29},
	{0x33,0x01},
	{0x34,0x00},
	{0x35,0x01},
	{0x36,0x00},
	{0x37,0x07},
	{0x38,0x17},
	{0x39,0x06},
	{0x3A,0x12},
	{0x3F,0x00},
	{0x40,0x02},
	{0x41,0xFF},
	{0x42,0x01},
	{0x46,0x2D},
	{0x47,0x0F},
	{0x48,0x3C},
	{0x49,0x00},
	{0x4A,0x1E},
	{0x4B,0x00},
	{0x4C,0x20},
	{0x4D,0x00},
	{0x4E,0x1A},
	{0x4F,0x14},
	{0x50,0x00},
	{0x51,0x10},
	{0x52,0x00},
	{0x5C,0x02},
	{0x5D,0x00},
	{0x5E,0x10},
	{0x5F,0x3F},
	{0x60,0x27},
	{0x61,0x28},
	{0x62,0x00},
	{0x63,0x03},
	{0x64,0xF7},
	{0x65,0x03},
	{0x66,0xD9},
	{0x67,0x03},
	{0x68,0x01},
	{0x69,0xC8},
	{0x6A,0x40},
	{0x6D,0x04},
	{0x6E,0x00},
	{0x6F,0x00},
	{0x70,0x80},
	{0x71,0x00},
	{0x72,0x00},
	{0x73,0x00},
	{0x74,0xF0},
	{0x75,0x00},
	{0x80,0x42},
	{0x81,0x44},
	{0x82,0x04},
	{0x83,0x20},
	{0x84,0x20},
	{0x85,0x00},
	{0x86,0x10},
	{0x87,0x00},
	{0x88,0x05},
	{0x89,0x18},
	{0x8A,0x10},
	{0x8B,0x01},
	{0x8C,0x37},
	{0x8D,0x00},
	{0x8E,0xF0},
	{0x8F,0x81},
	{0x90,0x06},
	{0x91,0x06},
	{0x92,0x1E},
	{0x93,0x0D},
	{0x94,0x0A},
	{0x95,0x0A},
	{0x96,0x0C},
	{0x97,0x05},
	{0x98,0x0A},
	{0x99,0x41},
	{0x9A,0x14},
	{0x9B,0x0A},
	{0x9C,0x3F},
	{0x9D,0x33},
	{0x9E,0xAE},
	{0x9F,0xF9},
	{0xA0,0x48},
	{0xA1,0x13},
	{0xA2,0x10},
	{0xA3,0x08},
	{0xA4,0x30},
	{0xA5,0x19},
	{0xA6,0x10},
	{0xA7,0x08},
	{0xA8,0x24},
	{0xA9,0x04},
	{0xAA,0x1E},
	{0xAB,0x1E},
	{0xCC,0x19},
	{0xCD,0x0B},
	{0xCE,0x13},
	{0xCF,0x64},
	{0xD0,0x21},
	{0xD1,0x0F},
	{0xD2,0x88},
	{0xE0,0x01},
	{0xE1,0x04},
	{0xE2,0x41},
	{0xE3,0xD6},
	{0xE4,0x00},
	{0xE5,0x0C},
	{0xE6,0x0A},
	{0xE7,0x00},
	{0xE8,0x00},
	{0xE9,0x00},
	{0xEE,0x07},
	{0xEF,0x01},
	{0x00,0x1E},
	{0x01,0x1E},
	{0x02,0x0F},
	{0x03,0x10},
	{0x04,0x02},
	{0x05,0x00},
	{0x06,0xB0},
	{0x07,0x04},
	{0x08,0x0D},
	{0x09,0x0E},
	{0x0A,0x9C},
	{0x0B,0x04},
	{0x0C,0x05},
	{0x0D,0x0F},
	{0x0E,0x02},
	{0x0F,0x12},
	{0x10,0x02},
	{0x11,0x02},
	{0x12,0x00},
	{0x13,0x01},
	{0x14,0x05},
	{0x15,0x07},
	{0x16,0x05},
	{0x17,0x07},
	{0x18,0x01},
	{0x19,0x04},
	{0x1A,0x05},
	{0x1B,0x0C},
	{0x1C,0x2A},
	{0x1D,0x01},
	{0x1E,0x00},
	{0x21,0x00},
	{0x22,0x00},
	{0x23,0x00},
	{0x25,0x01},
	{0x26,0x00},
	{0x27,0x39},
	{0x28,0x7F},
	{0x29,0x08},
	{0x30,0x03},
	{0x31,0x00},
	{0x32,0x1A},
	{0x33,0x1A},
	{0x34,0x07},
	{0x35,0x07},
	{0x36,0x01},
	{0x37,0xFF},
	{0x38,0x36},
	{0x39,0x07},
	{0x3A,0x00},
	{0x3E,0xFF},
	{0x3F,0x00},
	{0x40,0x77},
	{0x41,0x40},
	{0x42,0x00},
	{0x43,0x30},
	{0x44,0xA0},
	{0x45,0x5C},
	{0x46,0x00},
	{0x47,0x00},
	{0x48,0x58},
	{0x4A,0x1E},
	{0x4B,0x1E},
	{0x4C,0x00},
	{0x4D,0x00},
	{0x4E,0xA0},
	{0x4F,0x80},
	{0x50,0x00},
	{0x51,0x00},
	{0x52,0x00},
	{0x53,0x00},
	{0x54,0x00},
	{0x57,0x80},
	{0x59,0x10},
	{0x5A,0x08},
	{0x5B,0x94},
	{0x5C,0xE8},
	{0x5D,0x08},
	{0x5E,0x3D},
	{0x5F,0x99},
	{0x60,0x45},
	{0x61,0x40},
	{0x63,0x2D},
	{0x64,0x02},
	{0x65,0x96},
	{0x66,0x00},
	{0x67,0x97},
	{0x68,0x01},
	{0x69,0xCD},
	{0x6A,0x01},
	{0x6B,0xB0},
	{0x6C,0x04},
	{0x6D,0x2C},
	{0x6E,0x01},
	{0x6F,0x32},
	{0x71,0x00},
	{0x72,0x01},
	{0x73,0x35},
	{0x74,0x00},
	{0x75,0x33},
	{0x76,0x31},
	{0x77,0x01},
	{0x7C,0x84},
	{0x7D,0x03},
	{0x7E,0x01},
});



void PAJ7620::init(){
	PAJ7620_ASSERT(verify(), "paj7620 not found");

	for(const auto & item : initRegisterArray){
		writeReg(item.first, item.second);
	}

	/**
	 * Setting normal mode or gaming mode at BANK1 register 0x65/0x66 R_IDLE_TIME[15:0]
	 * T = 256/System CLK = 32us, 
	 * Ex:
	 * Far Mode: 1 report time = (77+R_IDLE_TIME)T
	 * Report rate 120 fps:
	 * R_IDLE_TIME=1/(120*T)-77=183
	 * 
	 * Report rate 240 fps:
	 * R_IDLE_TIME=1/(240*T)-77=53
	 * 
	 * Near Mode: 1 report time = (112+R_IDLE_TIME)T
	 * 
	 * Report rate 120 fps:
	 * R_IDLE_TIME=1/(120*T)-120=148
	 * 
	 * Report rate 240 fps:
	 * R_IDLE_TIME=1/(240*T)-112=18
	 * 
	 */
	selectBank(1);
	//paj7620WriteReg(0x65, 0xB7); // far mode 120 fps
	writeReg(0x65, 0x12);  // near mode 240 fps

	selectBank(0);
};