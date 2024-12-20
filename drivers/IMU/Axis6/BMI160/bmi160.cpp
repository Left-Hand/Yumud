#include "bmi160.hpp"


using namespace ymd::drivers;
using namespace ymd;


#ifdef BMI160_DEBUG
#undef BMI160_DEBUG
#define BMI160_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define BMI160_PANIC(...) PANIC(__VA_ARGS__)
#define BMI160_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define BMI160_DEBUG(...)
#define BMI160_PANIC(...)  PANIC()
#define BMI160_ASSERT(cond, ...) ASSERT(cond)
#endif

#define WRITE_REG(reg) this->writeReg(reg.address, reg);
#define READ_REG(reg) this->readReg(reg.address, reg);

void BMI160::requestData(const RegAddress addr, int16_t * datas, const size_t len){
    if(i2c_drv_){
        i2c_drv_->readMulti<int16_t>(uint8_t(addr), datas, len, LSB);
    }else if(spi_drv_){
        spi_drv_->writeSingle<uint8_t>(uint8_t(uint8_t(addr) | 0x80), CONT);
        spi_drv_->readMulti<uint8_t>(reinterpret_cast<uint8_t *>(datas), len * sizeof(int16_t));
    }else{
        BMI160_PANIC("no driver");
    }
}


void BMI160::init(){
    verify();

	// writeReg(0x40, 0b0'010'0110);		//ACC	ODR:25Hz		acc_bwp=3db(defult:acc_us=0b0)	
	// writeReg(0x41, 0x0C);		//Acc_range:16g
	// writeReg(0x42, 0b00'10'0110);		//Gro	ODR:25Hz		gro_bwp=3db	
	// writeReg(0x43, 0x03);		//Gro_range:250dps

    setAccOdr(AccOdr::_25);
    setAccRange(AccRange::_16G);
    setGyrOdr(Gyrdr::_25);
    setGyrRange(GyrRange::_250deg);
	//FIFO  Config
	writeReg(0x47, 0xfe);		//enable

    
	//Set PMU mode	Register(0x7E) CMD		attention the command
	setPmuMode(PmuType::ACC, PmuMode::NORMAL);		//Acc normal mode
	setPmuMode(PmuType::GYR, PmuMode::NORMAL);		//Gro normal mode
	//check the PMU_status	Register(0x03) 
    delay(20);

	BMI160_ASSERT(getPmuMode(PmuType::ACC) == PmuMode::NORMAL, "acc pmu mode verify failed");
    BMI160_ASSERT(getPmuMode(PmuType::GYR) == PmuMode::NORMAL, "gyr pmu mode verify failed");
}

void BMI160::update(){
    requestData(acc_reg.acc_address, &acc_reg.x, 3);
    requestData(gyr_reg.gyr_address, &gyr_reg.y, 3);
}

bool BMI160::verify(){
    uint8_t dummy;
    readReg(0x7f, dummy);
    readReg(chip_id_reg.address, chip_id_reg.data);
    bool passed = chip_id_reg.data == chip_id_reg.correct;
    BMI160_ASSERT(passed, "chip id verify failed");
    return passed;
}

void BMI160::reset(){
    writeCommand(uint8_t(Command::SOFT_RESET));
}

std::tuple<real_t, real_t, real_t> BMI160::getAcc(){
    auto conv = [&](const int16_t x) -> real_t{
        return s16_to_uni(x) * acc_scale;
    };
    
    return {
        conv(acc_reg.x),
        conv(acc_reg.y),
        conv(acc_reg.z)
    };
}

std::tuple<real_t, real_t, real_t> BMI160::getGyr(){
    auto conv = [&](const int16_t x) -> real_t{
        return s16_to_uni(x) * gyr_scale;
    };
    
    return {
        conv(gyr_reg.x),
        conv(gyr_reg.y),
        conv(gyr_reg.z)
    };
}

void BMI160::setPmuMode(const PmuType pmu, const PmuMode mode){
    switch(pmu){
        case PmuType::ACC:
            writeCommand(uint8_t(Command::ACC_SET_PMU) | uint8_t(mode));
            break;
        case PmuType::GYR:
            writeCommand(uint8_t(Command::GYR_SET_PMU) | uint8_t(mode));
            break;
        case PmuType::MAG:
            writeCommand(uint8_t(Command::MAG_SET_PMU) | uint8_t(mode));
            break;
    }
}

BMI160::PmuMode BMI160::getPmuMode(const PmuType type){
    auto & reg = pmu_status_reg;
    READ_REG(reg);

    switch(type){
        default:
        case PmuType::ACC:  return PmuMode(reg.acc_pmu_status);
        case PmuType::GYR: return PmuMode(reg.gyr_pmu_status);
        case PmuType::MAG:  return PmuMode(reg.mag_pmu_status);
    }
}

void BMI160::setAccOdr(const AccOdr odr){
    auto & reg = acc_conf_reg;
    reg.acc_odr = uint8_t(odr);
    reg.acc_bwp = 0b010;
    WRITE_REG(reg);
}

void BMI160::setAccRange(const AccRange range){
    auto & reg = acc_range_reg;
    reg.acc_range = uint8_t(range);
    WRITE_REG(reg);
    this->acc_scale = this->calculateAccScale(range);
}

void BMI160::setGyrOdr(const Gyrdr odr){
    auto & reg =  gyr_conf_reg;
    reg.gyr_odr = uint8_t(odr);
    reg.gyr_bwp = 0b010;
    WRITE_REG(reg);

}

void BMI160::setGyrRange(const GyrRange range){
    auto & reg = gyr_range_reg;
    reg.gyr_range = uint8_t(range);
    WRITE_REG(reg);
    this->gyr_scale = this->calculateGyrScale(range);
}


real_t BMI160::calculateAccScale(const AccRange range){
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

real_t BMI160::calculateGyrScale(const GyrRange range){
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