#include "bmi160.hpp"


using namespace ymd::drivers;
using namespace ymd;


#define WRITE_REG(reg) this->writeReg(reg.address, reg);
#define READ_REG(reg) this->readReg(reg.address, reg);

void BMI160::writeReg(const uint8_t addr, const uint8_t data){
    if(i2c_drv_){
        i2c_drv_->writeReg(addr, data, MSB);
    }else if(spi_drv_){
        spi_drv_->writeSingle(uint8_t(addr), CONT);
        spi_drv_->writeSingle(data);
    }else{
        BMI160_PANIC("no driver");
    }
}

void BMI160::readReg(const RegAddress addr, uint8_t & data){
    if(i2c_drv_){
        i2c_drv_->readReg((uint8_t)addr, data, MSB);
    }else if(spi_drv_){
        spi_drv_->writeSingle(uint8_t(uint8_t(addr) | 0x80), CONT);
        spi_drv_->readSingle(data);
    }else{
        BMI160_PANIC("no driver");
    }
}

void BMI160::requestData(const RegAddress addr, void * datas, const size_t len){
    if(i2c_drv_){
        i2c_drv_->readMulti<uint8_t>(uint8_t(addr), reinterpret_cast<uint8_t *>(datas), len, LSB);
    }else if(spi_drv_){
        spi_drv_->writeSingle<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT);
        spi_drv_->readMulti<uint8_t>(reinterpret_cast<uint8_t *>(datas), len);
    }else{
        BMI160_PANIC("no driver");
    }
}


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
    auto conv = [&](const uint16_t x) -> real_t{
        real_t ret;
        // u16_to_uni
        return  1;
    };
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
    //TODO
    return PmuMode::SUSPEND;
}

void BMI160::setAccelOdr(const AccOdr odr){
    auto & reg = acc_conf_reg;
    reg.acc_odr = uint8_t(odr);
    WRITE_REG(reg);
}

void BMI160::setAccelRange(const AccRange range){
    auto & reg = acc_range_reg;
    reg.acc_range = uint8_t(range);
    WRITE_REG(reg);
    this->acc_scale = this->calculateAccelScale(range);
}

void BMI160::setGyroOdr(const GyrOdr odr){
    auto & reg =  gyr_conf_reg;
    reg.gyr_odr = uint8_t(odr);
    WRITE_REG(reg);

}

void BMI160::setGyroRange(const GyrRange range){
    auto & reg = gyr_range_reg;
    reg.gyr_range = uint8_t(range);
    WRITE_REG(reg);
    this->gyr_scale = this->calculateGyroScale(range);
}


real_t BMI160::calculateAccelScale(const AccRange range){
    scexpr auto g = real_t(9.806);
    switch(range){
        default:
        case AccRange::_2G:
            return g * 2;
        case AccRange::_4G:
            return g * 4;
        case AccRange::_8G:
            return g * 8;
        case AccRange::_16G:
            return g * 16;
    }
}

real_t BMI160::calculateGyroScale(const GyrRange range){
    scexpr auto base125 = real_t(ANGLE2RAD(125.0));
    switch(range){
        default:
        case GyrRange::_125deg:
            return base125;
        case GyrRange::_250deg:
            return base125 * 2;
        case GyrRange::_500deg:
            return base125 * 4;
        case GyrRange::_1000deg:
            return base125 * 8;
        case GyrRange::_2000deg:
            return base125 * 16;
    }
}