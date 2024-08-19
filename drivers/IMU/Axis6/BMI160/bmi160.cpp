#include "bmi160.hpp"

void BMI160::init(){
	uint8_t ch = 0;
	writeReg(0x40, 0x26);		//ACC	ODR:25Hz		acc_bwp=3db(defult:acc_us=0b0)	
	writeReg(0x41, 0x0C);		//Acc_range:16g
	writeReg(0x42, 0x26);		//Gro	ODR:25Hz		gro_bwp=3db	
	writeReg(0x43, 0x03);		//Gro_range:250dps
	//FIFO  Config
	writeReg(0x47, 0xfe);		//enable
	//Set PMU mode	Register(0x7E) CMD		attention the command
	writeReg(0x7E, 11);		//Acc normal mode
	writeReg(0x7E, 0x15);		//Gro normal mode
	//check the PMU_status	Register(0x03) 

	readReg(0x03,ch);
	if (ch == 0x14){
		DEBUG_PRINTLN("sensor is Normal \r\n");
	}else{
		DEBUG_PRINTLN("sensor is bad \r\n");
    }
}

void BMI160::update(){

}

bool BMI160::check(){
    uint8_t dummy;
    readReg(0x00, dummy);
    BMI160_DEBUG(bool(portA[0]), bool(portA[15]));
    return false;
}

std::tuple<real_t, real_t, real_t> BMI160::getAccel(){
    return {0,0,0};
}

std::tuple<real_t, real_t, real_t> BMI160::getGyro(){
    return {0,0,0};
}