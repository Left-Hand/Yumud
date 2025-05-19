#include "TCA8418.hpp"


/*
* Arduino Library for TCA8418 I2C keyboard
*
* Copyright (C) 2015 RF Designs. All rights reserved.
*
* Author: Bob Frady <rfdesigns@live.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public
* License v2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 021110-1307, USA.
*
* If you can't comply with GPLv2, alternative licensing terms may be
* arranged. Please contact RF Designs <rfdesigns@live.com> for proprietary
* alternative licensing inquiries.
*/


using namespace ymd;
using namespace ymd::drivers;
using Error = TCA8418::Error;


static constexpr uint8_t KEYPAD = 			    0x37;
static constexpr uint8_t GPIO = 			    0x38;
static constexpr uint8_t EDGE = 			    0x39;
static constexpr uint8_t LEVEL = 			    0x40;
static constexpr uint8_t INTERRUPT = 			0x41;
static constexpr uint8_t NOINTERRUPT = 			0x42;
static constexpr uint8_t FIFO = 			    0x43;
static constexpr uint8_t NOFIFO = 			    0x44;
static constexpr uint8_t DEBOUNCE = 			0x45;
static constexpr uint8_t NODEBOUNCE = 			0x46;

#ifndef I2CTIMEOUT
static constexpr uint8_t I2CTIMEOUT = 			100;
#endif

/* TCA8418 hardware limits */
static constexpr uint8_t TCA8418_MAX_ROWS =     8;
static constexpr uint8_t TCA8418_MAX_COLS =     10;

/* TCA8418 register offsets */
static constexpr uint8_t REG_CFG = 			        0x01;
static constexpr uint8_t REG_INT_STAT = 			0x02;
static constexpr uint8_t REG_KEY_LCK_EC = 			0x03;
static constexpr uint8_t REG_KEY_EVENT_A = 			0x04;
static constexpr uint8_t REG_KEY_EVENT_B = 			0x05;
static constexpr uint8_t REG_KEY_EVENT_C = 			0x06;
static constexpr uint8_t REG_KEY_EVENT_D = 			0x07;
static constexpr uint8_t REG_KEY_EVENT_E = 			0x08;
static constexpr uint8_t REG_KEY_EVENT_F = 			0x09;
static constexpr uint8_t REG_KEY_EVENT_G = 			0x0A;
static constexpr uint8_t REG_KEY_EVENT_H = 			0x0B;
static constexpr uint8_t REG_KEY_EVENT_I = 			0x0C;
static constexpr uint8_t REG_KEY_EVENT_J = 			0x0D;
static constexpr uint8_t REG_KP_LCK_TIMER = 		0x0E;
static constexpr uint8_t REG_UNLOCK1 = 			    0x0F;
static constexpr uint8_t REG_UNLOCK2 = 			    0x10;
static constexpr uint8_t REG_GPIO_INT_STAT1 = 		0x11;
static constexpr uint8_t REG_GPIO_INT_STAT2 = 		0x12;
static constexpr uint8_t REG_GPIO_INT_STAT3 = 		0x13;
static constexpr uint8_t REG_GPIO_DAT_STAT1 = 		0x14;
static constexpr uint8_t REG_GPIO_DAT_STAT2 = 		0x15;
static constexpr uint8_t REG_GPIO_DAT_STAT3 = 		0x16;
static constexpr uint8_t REG_GPIO_DAT_OUT1 = 		0x17;
static constexpr uint8_t REG_GPIO_DAT_OUT2 = 		0x18;
static constexpr uint8_t REG_GPIO_DAT_OUT3 = 		0x19;
static constexpr uint8_t REG_GPIO_INT_EN1 = 		0x1A;
static constexpr uint8_t REG_GPIO_INT_EN2 = 		0x1B;
static constexpr uint8_t REG_GPIO_INT_EN3 = 		0x1C;
static constexpr uint8_t REG_KP_GPIO1 = 			0x1D;
static constexpr uint8_t REG_KP_GPIO2 = 			0x1E;
static constexpr uint8_t REG_KP_GPIO3 = 			0x1F;
static constexpr uint8_t REG_GPI_EM1 = 			    0x20;
static constexpr uint8_t REG_GPI_EM2 = 			    0x21;
static constexpr uint8_t REG_GPI_EM3 = 			    0x22;
static constexpr uint8_t REG_GPIO_DIR1 = 			0x23;
static constexpr uint8_t REG_GPIO_DIR2 = 			0x24;
static constexpr uint8_t REG_GPIO_DIR3 = 			0x25;
static constexpr uint8_t REG_GPIO_INT_LVL1 = 	    0x26;
static constexpr uint8_t REG_GPIO_INT_LVL2 = 	    0x27;
static constexpr uint8_t REG_GPIO_INT_LVL3 = 	    0x28;
static constexpr uint8_t REG_DEBOUNCE_DIS1 = 	    0x29;
static constexpr uint8_t REG_DEBOUNCE_DIS2 = 	    0x2A;
static constexpr uint8_t REG_DEBOUNCE_DIS3 = 	    0x2B;
static constexpr uint8_t REG_GPIO_PULL1 = 			0x2C;
static constexpr uint8_t REG_GPIO_PULL2 = 			0x2D;
static constexpr uint8_t REG_GPIO_PULL3 = 			0x2E;

/* TCA8418 bit definitions */
static constexpr uint8_t CFG_AI = 			        0x80;
static constexpr uint8_t CFG_GPI_E_CFG = 			0x40;
static constexpr uint8_t CFG_OVR_FLOW_M = 			0x20;
static constexpr uint8_t CFG_INT_CFG = 			    0x10;
static constexpr uint8_t CFG_OVR_FLOW_IEN =         0x08;
static constexpr uint8_t CFG_K_LCK_IEN = 			0x04;
static constexpr uint8_t CFG_GPI_IEN = 			    0x02;
static constexpr uint8_t CFG_KE_IEN = 			    0x01;

static constexpr uint8_t INT_STAT_CAD_INT =         0x10;
static constexpr uint8_t INT_STAT_OVR_FLOW_INT =    0x08;
static constexpr uint8_t INT_STAT_K_LCK_INT =       0x04;
static constexpr uint8_t INT_STAT_GPI_INT =         0x02;
static constexpr uint8_t INT_STAT_K_INT = 			0x01;

/* TCA8418 register masks */
static constexpr uint8_t KEY_LCK_EC_KEC = 			0x7;
static constexpr uint8_t KEY_EVENT_CODE = 			0x7f;
static constexpr uint8_t KEY_EVENT_VALUE = 			0x80;

uint8_t TCA8418::readKeypad(void)
{
    return(getKeyEvent());
}


/*
* Configure the TCA8418 for keypad operation
*/
bool TCA8418::configureKeys(uint8_t rows, uint16_t cols, uint8_t config)
{
  //Pins all default to GPIO. pinMode(x, KEYPAD); may be used for individual pins.
    writeByte(rows, REG_KP_GPIO1);
    
    uint8_t col_tmp;
    col_tmp = (uint8_t)(0xff & cols);
    writeByte(col_tmp, REG_KP_GPIO2);
    col_tmp = (uint8_t)(0x03 & (cols>>8));
    writeByte(col_tmp, REG_KP_GPIO3);
    
    config |= CFG_AI;
    writeByte(config, REG_CFG);

    TODO();
    return false;
}


// #ifdef TCA8418_INTERRUPT_SUPPORT
// void TCA8418::enableInterrupt(uint8_t pin, void(*selfCheckFunction)(void)) {

//   _pcintPin = pin;
  
// #if ARDUINO >= 100
//   ::pinMode(pin, INPUT_PULLUP);
// #else
//   ::pinMode(pin, INPUT);
//   ::digitalWrite(pin, HIGH);
// #endif

//   PCattachInterrupt(pin, selfCheckFunction, FALLING);
// }

// void TCA8418::disableInterrupt() {
//   PCdetachInterrupt(_pcintPin);
// }

// void TCA8418::pinInterruptMode(uint32_t pin, uint8_t mode, uint8_t level, uint8_t fifo) {
//   uint32_t intSetting, levelSetting, eventmodeSetting;

	
//   read3Bytes((uint32_t *)&intSetting, REG_GPIO_INT_EN1);
//   read3Bytes((uint32_t *)&levelSetting, REG_GPIO_INT_LVL1);
//   read3Bytes((uint32_t *)&eventmodeSetting, REG_GPI_EM1);

//   switch(mode) {
//     case INTERRUPT:
// 	  bitSet(intSetting, pin);
// 	  break;
// 	case NOINTERRUPT:
// 	  bitClear(intSetting, pin);
// 	  break;
// 	default:
// 		break;
//   }
  
//   switch(level) {
//     case LOW:
// 	  bitClear(levelSetting, pin);
// 	  break;
// 	case HIGH:
// 	  bitSet(levelSetting, pin);
// 	  break;
// 	default:
// 	  break;
//   }
  
//   switch(fifo) {
//     case FIFO:
// 	  bitSet(eventmodeSetting, pin);
// 	  break;
// 	case NOFIFO:
// 	  bitClear(eventmodeSetting, pin);
// 	  break;
// 	default:
// 	  break;
//   }
  
//   write3Bytes((uint32_t)intSetting, REG_GPIO_INT_EN1);
//   write3Bytes((uint32_t)levelSetting, REG_GPIO_INT_LVL1);
//   write3Bytes((uint32_t)eventmodeSetting, REG_GPI_EM1);
  
// }

// void TCA8418::pinInterruptMode(uint32_t pin, uint8_t mode) {
//   pinInterruptMode(pin, mode, 0, 0);
// }

// #endif

void TCA8418::readGPIO() {

#ifdef TCA8418_INTERRUPT_SUPPORT
	/* Store old _PIN value */
	_oldPIN = _PIN;
#endif
	
	read3Bytes((uint32_t *)&_PORT, REG_GPIO_DAT_OUT1);  //Read Data OUT Registers
	read3Bytes((uint32_t *)&_PIN, REG_GPIO_DAT_STAT1);	//Read Data STATUS Registers
	read3Bytes((uint32_t *)&_DDR, REG_GPIO_DIR1);		//Read Data DIRECTION Registers
	read3Bytes((uint32_t *)&_PKG, REG_KP_GPIO1);		//Read Keypad/GPIO SELECTION Registers
	read3Bytes((uint32_t *)&_PUR, REG_GPIO_PULL1);		//Read KPull-Up RESISTOR Registers
}

void TCA8418::updateGPIO() {

	write3Bytes((uint32_t)_PORT, REG_GPIO_DAT_OUT1);  	//Write Data OUT Registers
	write3Bytes((uint32_t)_DDR, REG_GPIO_DIR1);			//Write Data DIRECTION Registers
	write3Bytes((uint32_t)_PKG, REG_KP_GPIO1);			//Write Keypad/GPIO SELECTION Registers
	write3Bytes((uint32_t)_PUR, REG_GPIO_PULL1);		//Write Pull-Up RESISTOR Registers
}


uint8_t TCA8418::getInterruptStatus(void) {
    uint8_t status;
    
    readByte(&status, REG_INT_STAT);
    return(status & 0x0F);
}

void TCA8418::clearInterruptStatus(uint8_t flags) {

    flags &= 0x0F;
    writeByte(flags, REG_INT_STAT);
}

void TCA8418::clearInterruptStatus(void) {
    clearInterruptStatus(0x0F);
}

uint8_t TCA8418::getKeyEvent(uint8_t event) {
    uint8_t keycode;
    
    if (event > 9)
        return 0x00;
        
    readByte(&keycode, (REG_KEY_EVENT_A+event));
    return(keycode);
}

uint8_t TCA8418::getKeyEvent(void) {
    return(getKeyEvent(0));
}

uint8_t TCA8418::getKeyEventCount(void) {
    uint8_t count;
    
    readByte(&count, REG_KEY_LCK_EC);
    return(count & 0x0F);
}

uint32_t TCA8418::getGPIOInterrupt(void) {
    uint32_t Ints;
    
    union {
        uint32_t val;
        uint8_t arr[4];
    } IntU;
    
    readByte(&IntU.arr[2], REG_GPIO_INT_STAT3);
    readByte(&IntU.arr[1], REG_GPIO_INT_STAT2);
    readByte(&IntU.arr[0], REG_GPIO_INT_STAT1);

    Ints = IntU.val;
    return(Ints);
}

bool TCA8418::isKeyDown(uint8_t key) {
    if(key & 0x80)
        return true;
    else
        return false;
}

bool TCA8418::getKey(uint8_t *key) {
    uint8_t tmpkey;
    
    tmpkey = readKeypad();
    *key = *key & 0x7F;
    
    return(isKeyDown(tmpkey));
}

uint8_t TCA8418::getKey(void) {
    return(readKeypad() & 0x7F);
}

void TCA8418::writeByte(uint8_t data, uint8_t reg) {
    TODO();
    // Wire.beginTransmission(_address);
    // I2CWRITE((uint8_t) reg);
    
    // I2CWRITE((uint8_t) data);
    // Wire.endTransmission();

    return;
}

bool TCA8418::readByte(uint8_t *data, uint8_t reg) {
    TODO();
    // Wire.beginTransmission(_address);
    // I2CWRITE((uint8_t) reg);
    // Wire.endTransmission();
    // uint8_t timeout=0;

    // Wire.requestFrom(_address, (uint8_t) 0x01);
    // while(Wire.available() < 1) {
    //     timeout++;
    //     if(timeout > I2CTIMEOUT) {
    //     return(true);
    //     }
    //     clock::delay(1ms);
    // } 			// Experimental

    // *data = I2CREAD();

    return(false);
}

void TCA8418::write3Bytes(uint32_t data, uint8_t reg) {
    TODO();
    // union
    // {
    //   uint8_t b[4];
    //   uint32_t w;
    // } datau;
    
    // datau.w = data;
  
    // Wire.beginTransmission(_address);
    // I2CWRITE((uint8_t) reg);
    
    // I2CWRITE((uint8_t) datau.b[0]);
    // I2CWRITE((uint8_t) datau.b[1]);
    // I2CWRITE((uint8_t) datau.b[2]);
    
    // Wire.endTransmission();
    return;
  }
  
  bool TCA8418::read3Bytes(uint32_t *data, uint8_t reg) {
    TODO();
    // union
    // {
    //   uint8_t b[4];
    //   uint32_t w;
    // } datau;
    
    // datau.w = *data;
  
    // Wire.beginTransmission(_address);
    // I2CWRITE((uint8_t) reg);
    // Wire.endTransmission();
    // uint8_t timeout=0;
    
    // Wire.requestFrom(_address, (uint8_t) 0x03);
    // while(Wire.available() < 3) {
    //   timeout++;
    //   if(timeout > I2CTIMEOUT) {
    //     return(true);
    //   }
    //   clock::delay(1ms);
    // } 		//Experimental
    
    // datau.b[0] = I2CREAD();
    // datau.b[1] = I2CREAD();
    // datau.b[2] = I2CREAD();
    
    // *data = datau.w;
    
  
  return(false);
  }