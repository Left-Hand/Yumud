#include "bmi160.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

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

using Error = BMI160::Error;

Result<void, Error> BMI160::init(){
    const auto res = reset()
    | validate()
    | set_acc_odr(AccOdr::_800)
    | set_acc_range(AccRange::_8G)
    | set_gyr_odr(GyrOdr::_800)
    | set_gyr_range(GyrRange::_1000deg)
	| phy_.write_reg(0x47, 0xfe)		//enable
	| set_pmu_mode(PmuType::ACC, PmuMode::NORMAL)		//Acc normal mode
	| set_pmu_mode(PmuType::GYR, PmuMode::NORMAL)		//Gro normal mode
	//check the PMU_status	Register(0x03) 
    ;
    delay(1);
	BMI160_ASSERT(get_pmu_mode(PmuType::ACC) == PmuMode::NORMAL, "acc pmu mode verify failed");
    BMI160_ASSERT(get_pmu_mode(PmuType::GYR) == PmuMode::NORMAL, "gyr pmu mode verify failed");

    return res;
}

Result<void, Error> BMI160::update(){
    return phy_.read_burst(acc_reg.acc_address, &acc_reg.x, 3) 
        | phy_.read_burst(gyr_reg.gyr_address, &gyr_reg.x, 3);
}

Result<void, Error> BMI160::validate(){
    uint8_t dummy;
    const auto err = phy_.read_reg(0x7f, dummy) 
        | phy_.read_reg(chip_id_reg.address, chip_id_reg.data);
    bool passed = chip_id_reg.data == chip_id_reg.correct;
    BMI160_ASSERT(passed, "chip id verify failed");
    if(passed or err.is_err()) return Ok();
    else return err;
}

Result<void, Error> BMI160::reset(){
    return phy_.write_command(uint8_t(Command::SOFT_RESET));
}

Option<Vector3_t<q24>> BMI160::get_acc(){
    auto conv = [&](const int16_t x) -> real_t{
        return s16_to_uni(x) * acc_scale;
    };
    
    return Some{Vector3_t<q24>{
        conv(acc_reg.x),
        conv(acc_reg.y),
        conv(acc_reg.z)
    }};
}

Option<Vector3_t<q24>> BMI160::get_gyr(){
    auto conv = [&](const int16_t x) -> real_t{
        return s16_to_uni(x) * gyr_scale;
    };
    
    return Some{Vector3_t<q24>{
        conv(gyr_reg.x),
        conv(gyr_reg.y),
        conv(gyr_reg.z)
    }};
}

Result<void, Error> BMI160::set_pmu_mode(const PmuType pmu, const PmuMode mode){
    switch(pmu){
        case PmuType::ACC:
            return phy_.write_command(uint8_t(Command::ACC_SET_PMU) | uint8_t(mode));
        case PmuType::GYR:
            return phy_.write_command(uint8_t(Command::GYR_SET_PMU) | uint8_t(mode));
        case PmuType::MAG:
            return phy_.write_command(uint8_t(Command::MAG_SET_PMU) | uint8_t(mode));
    }

    __builtin_unreachable();
}

BMI160::PmuMode BMI160::get_pmu_mode(const PmuType type){
    auto & reg = pmu_status_reg;
    phy_.read_regs(reg).unwrap();

    switch(type){
        default:
        case PmuType::ACC:  return PmuMode(reg.acc_pmu_status);
        case PmuType::GYR: return PmuMode(reg.gyr_pmu_status);
        case PmuType::MAG:  return PmuMode(reg.mag_pmu_status);
    }
}

Result<void, Error> BMI160::set_acc_odr(const AccOdr odr){
    auto & reg = acc_conf_reg;
    reg.acc_odr = uint8_t(odr);
    reg.acc_bwp = 0b010;
    return phy_.write_regs(reg);
}

Result<void, Error> BMI160::set_acc_range(const AccRange range){
    this->acc_scale = this->calculate_acc_scale(range);
    auto & reg = acc_range_reg;
    reg.acc_range = uint8_t(range);
    return phy_.write_regs(reg);
}

Result<void, Error> BMI160::set_gyr_odr(const GyrOdr odr){
    auto & reg =  gyr_conf_reg;
    reg.gyr_odr = uint8_t(odr);
    reg.gyr_bwp = 0b010;
    return phy_.write_regs(reg);

}
Result<void, Error> BMI160::set_gyr_range(const GyrRange range){
    this->gyr_scale = this->calculate_gyr_scale(range);
    auto & reg = gyr_range_reg;
    reg.gyr_range = uint8_t(range);
    return phy_.write_regs(reg);
}



real_t BMI160::calculate_acc_scale(const AccRange range){
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

real_t BMI160::calculate_gyr_scale(const GyrRange range){
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