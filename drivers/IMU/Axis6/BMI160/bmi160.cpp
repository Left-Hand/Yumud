#include "bmi160.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;


#define BMI160_DEBUG_EN
// #define SELFTEST_EN

#ifdef BMI160_DEBUG_EN
#define BMI160_TODO(...) TODO()
#define BMI160_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define BMI160_PANIC(...) PANIC{__VA_ARGS__}
#define BMI160_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}


#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    PANIC{#x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define BMI160_DEBUG(...)
#define BMI160_TODO(...) PANIC_NSRC()
#define BMI160_PANIC(...)  PANIC_NSRC()
#define BMI160_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif

using Error = BMI160::Error;

template<typename T = void>
using IResult = Result<T, Error>;

static constexpr size_t MAX_PMU_SETUP_RETRY_TIMES = 60;


IResult<> BMI160::init(const Config & cfg){
    if(const auto res = reset();
        res.is_err()) return res;

    clock::delay(1ms);

    if(const auto res = validate()
        ;res.is_err()) return res;

    {
        //power up acc

        if(const auto res = set_pmu_mode(PmuType::ACC, PmuMode::NORMAL)		//Acc normal mode
            ;res.is_err()) return res;
        clock::delay(1ms);

        //wait for power up acc
        if(const auto res = 
        retry(MAX_PMU_SETUP_RETRY_TIMES, [this] -> IResult<>{
            if(const auto res = (get_pmu_mode(PmuType::ACC));
                res.is_err()) return Err(res.unwrap_err());
            else if (res.unwrap() != PmuMode::NORMAL){
                return Err(Error::AccCantSetup);
            }
            return Ok();
        }, []{clock::delay(1ms);}); 
            
        res.is_err()) return Err(res.unwrap_err());
    }
    
    #ifdef SELFTEST_EN
    if(const auto res = self_test_acc();
        res.is_err()) return res;
    #endif

    {
        //power up gyr

        if(const auto res = set_pmu_mode(PmuType::GYR, PmuMode::NORMAL)		//Gro normal mode
            ;res.is_err()) return res;
        clock::delay(1ms);

        //wait for power up gyr

        if(const auto res = 
        retry(MAX_PMU_SETUP_RETRY_TIMES, [this] -> IResult<>{
            if(const auto res = (get_pmu_mode(PmuType::GYR));
                res.is_err()) return Err(res.unwrap_err());
            else if (res.unwrap() != PmuMode::NORMAL){
                return Err(Error::GyrCantSetup);
            }
            return Ok();
        }, []{clock::delay(1ms);}); 
            
        res.is_err()) return Err(res.unwrap_err());
    }

    #ifdef SELFTEST_EN
    if(const auto res = self_test_gyr();
        res.is_err()) return res;
    #endif

    if(const auto res = set_gyr_odr(cfg.gyr_odr)
        ;res.is_err()) return res;
    if(const auto res = set_gyr_fs(cfg.gyr_fs)
        ;res.is_err()) return res;

    if(const auto res = set_acc_odr(cfg.acc_odr)
        ;res.is_err()) return res;
    if(const auto res = set_acc_fs(cfg.acc_fs)
        ;res.is_err()) return res;




    clock::delay(1ms);
    return Ok();
}


IResult<> BMI160::self_test_acc(){
    // page44 2.8.1 
    {
        auto reg = RegCopy(regs_.acc_conf_reg);
        reg.acc_odr = AccOdr::_1600Hz;
        reg.acc_bwp= 2;
        reg.acc_us = 0;
        if(const auto res = phy_.write_reg(reg);
            res.is_err()) return res;
    }

    {
        auto reg = RegCopy(regs_.self_test);
        reg.acc_self_test_en = 1;
        if(const auto res = phy_.write_reg(reg);
            res.is_err()) return res;
    }

    // https://www.wpgdadatong.com/blog/detail/42050
    // 參考暫存器0x6D， 
    // Enable acc_self_test_amp bit，再改變acc_self_test_sign 和 acc_self_test-enable bits，
    // 致能 self -test 需等待50 ms，後讀取加速度輸出資料分別檢測+X、-X、+Y、 -Y、 +Z、 -Z 的輸出資料，
    // 同一軸的差值需達到下列表列中的標準，才能判斷MEMS 是正常的。
    TODO();

    {
        auto reg = RegCopy(regs_.self_test);
        reg.acc_self_test_en = 0;
        if(const auto res = phy_.write_reg(reg);
            res.is_err()) return res;
    }

    if(const auto res = reset();
        res.is_err()) return res;

    clock::delay(1ms);

    return Ok();
}

IResult<> BMI160::self_test_gyr(){

    {
        auto reg = RegCopy(regs_.self_test);
        reg.gyr_self_test_en = 1;
        if(const auto res = phy_.write_reg(reg);
            res.is_err()) return res;
    }

    if(const auto res = 
        retry(MAX_PMU_SETUP_RETRY_TIMES, [this] -> IResult<>{
            auto & reg = regs_.status_reg;
            if(const auto res = (phy_.read_regs(reg));
                res.is_err()) return Err(res.unwrap_err());

            if (reg.gyr_self_test_ok != 1){
                return Err(Error::GyrCantSetup);
            }
            return Ok();
        }, []{clock::delay(1ms);}); 
        
        res.is_err()) return Err(res.unwrap_err());

    {
        auto reg = RegCopy(regs_.self_test);
        reg.gyr_self_test_en = 1;
        if(const auto res = phy_.write_reg(reg);
            res.is_err()) return res;
    }

    return Ok();
}
IResult<> BMI160::update(){
    return phy_.read_burst(regs_.acc_reg.acc_address, &regs_.acc_reg.x, 3) 
        | phy_.read_burst(regs_.gyr_reg.gyr_address, &regs_.gyr_reg.x, 3);
}

IResult<> BMI160::validate(){
    uint8_t dummy;
    if(const auto res = phy_.read_reg(0x7f, dummy);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = phy_.read_reg(regs_.chip_id_reg.address, regs_.chip_id_reg.data);
        res.is_err()) return Err(res.unwrap_err());

    if(regs_.chip_id_reg.data != regs_.chip_id_reg.CORRECT_ID)
        return CHECK_ERR(Err(Error::WrongWhoAmI), "read id is", regs_.chip_id_reg.data);

    return Ok();
}

IResult<> BMI160::reset(){
    return phy_.write_command(uint8_t(Command::SOFT_RESET));
}

IResult<Vector3<q24>> BMI160::read_acc(){
    auto conv = [&](const int16_t x) -> real_t{
        return s16_to_uni(x) * acc_scale_;
    };
    
    return Ok{Vector3<q24>{
        conv(regs_.acc_reg.x),
        conv(regs_.acc_reg.y),
        conv(regs_.acc_reg.z)
    }};
}

IResult<Vector3<q24>> BMI160::read_gyr(){
    auto conv = [&](const int16_t x) -> real_t{
        return s16_to_uni(x) * gyr_scale_;
    };
    
    return Ok{Vector3<q24>{
        conv(regs_.gyr_reg.x),
        conv(regs_.gyr_reg.y),
        conv(regs_.gyr_reg.z)
    }};
}

IResult<> BMI160::set_pmu_mode(const PmuType pmu, const PmuMode mode){
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

IResult<BMI160::PmuMode> BMI160::get_pmu_mode(const PmuType type){
    auto & reg = regs_.pmu_status_reg;

    if(const auto res = phy_.read_regs(reg);
        res.is_err()) return Err(res.unwrap_err());

    switch(type){
        case PmuType::ACC:  return  Ok(PmuMode(reg.acc_pmu_status));
        case PmuType::GYR:  return  Ok(PmuMode(reg.gyr_pmu_status));
        case PmuType::MAG:  return  Ok(PmuMode(reg.mag_pmu_status));
        default: __builtin_unreachable();
    }
}

IResult<> BMI160::set_acc_odr(const AccOdr odr){
    auto reg = RegCopy(regs_.acc_conf_reg);
    reg.acc_odr = odr;
    reg.acc_bwp = 0b010;
    return phy_.write_reg(reg);
}

IResult<> BMI160::set_acc_fs(const AccFs fs){
    this->acc_scale_ = this->calculate_acc_scale(fs);
    auto reg = RegCopy(regs_.acc_fs_reg);
    reg.acc_fs = fs;
    return phy_.write_reg(reg);
}

IResult<> BMI160::set_gyr_odr(const GyrOdr odr){
    auto reg = RegCopy(regs_.gyr_conf_reg);
    reg.gyr_odr = odr;
    reg.gyr_bwp = 0b010;
    return phy_.write_reg(reg);

}
IResult<> BMI160::set_gyr_fs(const GyrFs fs){
    this->gyr_scale_ = this->calculate_gyr_scale(fs);
    auto reg = RegCopy(regs_.gyr_fs_reg);
    reg.gyr_fs = fs;
    return phy_.write_reg(reg);
}

