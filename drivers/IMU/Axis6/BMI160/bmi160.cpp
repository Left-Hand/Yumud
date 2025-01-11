#include "bmi160.hpp"
#include <array>

using namespace ymd::drivers;
using namespace ymd;

#define BMI160_DEBUG

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

void BMI160::init(){
    reset();
    delay(10);

    verify();

    setAccOdr(AccOdr::_800);
    setAccRange(AccRange::_8G);
    setGyrOdr(GyrOdr::_800);
    setGyrRange(GyrRange::_1000deg);
	//FIFO  Config
	writeReg(0x47, 0xfe);		//enable

    
	//Set PMU mode	Register(0x7E) CMD		attention the command
	setPmuMode(PmuType::ACC, PmuMode::NORMAL);		//Acc normal mode
	setPmuMode(PmuType::GYR, PmuMode::NORMAL);		//Gro normal mode
	//check the PMU_status	Register(0x03) 

    delay(2);

	BMI160_ASSERT(getPmuMode(PmuType::ACC) == PmuMode::NORMAL, "acc pmu mode verify failed");
    BMI160_ASSERT(getPmuMode(PmuType::GYR) == PmuMode::NORMAL, "gyr pmu mode verify failed");
}

void BMI160::update(){
    requestData(acc_reg.acc_address, &acc_reg.x, 3);
    requestData(gyr_reg.gyr_address, &gyr_reg.x, 3);
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

void BMI160::setGyrOdr(const GyrOdr odr){
    auto & reg =  gyr_conf_reg;
    reg.gyr_odr = uint8_t(odr);
    reg.gyr_bwp = 0b010;
    WRITE_REG(reg);

}
real_t BMI160::setGyrOdr(const real_t odr){
    scexpr std::array odr_map = {
        25, 50, 100, 200, 400, 800, 1600, 3200
    };

    auto it = std::lower_bound(odr_map.begin(), odr_map.end(), (odr));

    if (it != odr_map.end()) {
        auto index = std::distance(odr_map.begin(), it);
        setGyrOdr(static_cast<GyrOdr>(index + uint8_t(GyrOdr::_25)));
        return *it;
    }else {
        BMI160_PANIC("Invalid ODR value");
    }
    return 0;
}

real_t BMI160::setAccOdr(const real_t odr){
    scexpr std::array odr_map = {
        real_t(25.0/32),
        real_t(25.0/16),
        real_t(25.0/8),
        real_t(25.0/4),
        real_t(25.0/2),
        real_t(25), 
        real_t(50), 
        real_t(100), 
        real_t(200), 
        real_t(400), 
        real_t(800),
        real_t(1600)
    };

    auto it = std::lower_bound(odr_map.begin(), odr_map.end(), (odr));

    if (it != odr_map.end()) {
        auto index = std::distance(odr_map.begin(), it);
        setAccOdr(static_cast<AccOdr>(index + uint8_t(AccOdr::_25_32)));
        return *it;
    } else {
        BMI160_PANIC("Invalid ODR value");
    }
    return 0;
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