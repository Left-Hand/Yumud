#include "bmi160.hpp"



void BMI160::init(){
    verify();

	writeReg(0x40, 0x26);		//ACC	ODR:25Hz		acc_bwp=3db(defult:acc_us=0b0)	
	writeReg(0x41, 0x0C);		//Acc_range:16g
	writeReg(0x42, 0x26);		//Gro	ODR:25Hz		gro_bwp=3db	
	writeReg(0x43, 0x03);		//Gro_range:250dps
	//FIFO  Config
	writeReg(0x47, 0xfe);		//enable
	//Set PMU mode	Register(0x7E) CMD		attention the command
	setPmuMode(PmuType::ACC, PmuMode::NORMAL);		//Acc normal mode
	setPmuMode(PmuType::GYRO, PmuMode::NORMAL);		//Gro normal mode
	//check the PMU_status	Register(0x03) 
    delay(200);
	uint8_t ch = 0;
	readReg(0x03,ch);
    
	if (ch == 0x14){
		DEBUG_PRINTLN("sensor init succeed");
	}else{
		DEBUG_PRINTLN("sensor init failed");
    }
}

void BMI160::update(){
    requestData(accel_reg.acc_address, &accel_reg, sizeof(accel_reg));
    requestData(gyro_reg.gyro_address, &gyro_reg, sizeof(gyro_reg));
}

bool BMI160::verify(){
    uint8_t dummy;
    readReg(0x7f, dummy);
    readReg(chip_id_reg.address, chip_id_reg.data);
    bool passed = chip_id_reg.data == chip_id_reg.correct;
    if(passed){
        DEBUG_PRINTLN("sensor verify passed");
    }else{
        DEBUG_PRINTLN("sensor verify failed");
    }
    return passed;
}

void BMI160::reset(){
    writeCommand(uint8_t(Command::SOFT_RESET));
}

std::tuple<real_t, real_t, real_t> BMI160::getAccel(){
    return {accel_reg.x,accel_reg.y,accel_reg.z};
}

std::tuple<real_t, real_t, real_t> BMI160::getGyro(){
    return {gyro_reg.x, gyro_reg.y, gyro_reg.z};
}

void BMI160::setPmuMode(const PmuType pmu, const PmuMode mode){
    switch(pmu){
        case PmuType::ACC:
            writeCommand(uint8_t(Command::ACC_SET_PMU) | uint8_t(mode));
            break;
        case PmuType::GYRO:
            writeCommand(uint8_t(Command::GYRO_SET_PMU) | uint8_t(mode));
            break;
        case PmuType::MAG:
            writeCommand(uint8_t(Command::MAG_SET_PMU) | uint8_t(mode));
            break;
    }
}

BMI160::PmuMode BMI160::getPmuMode(const PmuType pmu){
    // readReg(pmu)
    return PmuMode::SUSPEND;
}