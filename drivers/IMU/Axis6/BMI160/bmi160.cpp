#include "bmi160.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;


// #define BMI160_DEBUG_EN
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
        unlikely(res.is_err())) return res;

    clock::delay(1ms);

    if(const auto res = validate()
        ;unlikely(res.is_err())) return res;

    {
        //power up acc
        //向命令寄存器写入0x11,使加速度计处于正常工作模式  
        if(const auto res = set_pmu_mode(PmuType::ACC, PmuMode::NORMAL)		//Acc normal mode
            ;unlikely(res.is_err())) return res;
        clock::delay(1ms);

        //wait for power up acc
        if(const auto pw_res = retry(MAX_PMU_SETUP_RETRY_TIMES, [this] -> IResult<>{
            if(const auto res = (get_pmu_mode(PmuType::ACC));
                unlikely(res.is_err())) return Err(res.unwrap_err());
            else if (res.unwrap() != PmuMode::NORMAL){
                return Err(Error::AccCantSetup);
            }
            return Ok();
        }, []{clock::delay(1ms);}); 
            
        unlikely(pw_res.is_err())) return Err(pw_res.unwrap_err());
    }
    
    #ifdef SELFTEST_EN
    if(const auto res = self_test_acc();
        unlikely(res.is_err())) return res;
    #endif

    {
        //power up gyr
        //向命令寄存器写入0x15,使陀螺仪处于正常工作模式 
        if(const auto res = set_pmu_mode(PmuType::GYR, PmuMode::NORMAL)		//Gro normal mode
            ;unlikely(res.is_err())) return res;
        clock::delay(1ms);

        //wait for power up gyr

        if(const auto res = retry(MAX_PMU_SETUP_RETRY_TIMES, [this] -> IResult<>{
            if(const auto _res = (get_pmu_mode(PmuType::GYR));
                unlikely(_res.is_err())) return Err(_res.unwrap_err());
            else if (_res.unwrap() != PmuMode::NORMAL){
                return Err(Error::GyrCantSetup);
            }
            return Ok();
        }, []{clock::delay(1ms);}); 
            
        unlikely(res.is_err())) return Err(res.unwrap_err());
    }

    #ifdef SELFTEST_EN
    if(const auto res = self_test_gyr();
        unlikely(res.is_err())) return res;
    #endif


    if(const auto res = set_acc_odr(cfg.acc_odr)
        ;unlikely(res.is_err())) return res;
    if(const auto res = set_acc_fs(cfg.acc_fs)
        ;unlikely(res.is_err())) return res;

    if(const auto res = set_gyr_odr(cfg.gyr_odr)
        ;unlikely(res.is_err())) return res;
    if(const auto res = set_gyr_fs(cfg.gyr_fs)
        ;unlikely(res.is_err())) return res;

    return Ok();
}


IResult<> BMI160::self_test_acc(){
    // page44 2.8.1 
    {
        auto reg = RegCopy(regs_.acc_conf);
        reg.acc_odr = AccOdr::_1600Hz;
        reg.acc_bwp= 2;
        reg.acc_us = 0;
        if(const auto res = write_reg(reg);
            unlikely(res.is_err())) return res;
    }

    {
        auto reg = RegCopy(regs_.self_test);
        reg.acc_self_test_en = 1;
        if(const auto res = write_reg(reg);
            unlikely(res.is_err())) return res;
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
        if(const auto res = write_reg(reg);
            unlikely(res.is_err())) return res;
    }

    if(const auto res = reset();
        unlikely(res.is_err())) return res;

    clock::delay(1ms);

    return Ok();
}

IResult<> BMI160::self_test_gyr(){

    {
        auto reg = RegCopy(regs_.self_test);
        reg.gyr_self_test_en = 1;
        if(const auto res = write_reg(reg);
            unlikely(res.is_err())) return res;
    }

    if(const auto res = 
        retry(MAX_PMU_SETUP_RETRY_TIMES, [this] -> IResult<>{
            auto & reg = regs_.status;
            if(const auto _res = (read_reg(reg));
                unlikely(_res.is_err())) return Err(_res.unwrap_err());

            if (reg.gyr_self_test_ok != 1){
                return Err(Error::GyrCantSetup);
            }
            return Ok();
        }, []{clock::delay(1ms);}); 
        
        unlikely(res.is_err())) return Err(res.unwrap_err());

    {
        auto reg = RegCopy(regs_.self_test);
        reg.gyr_self_test_en = 1;
        if(const auto res = write_reg(reg);
            unlikely(res.is_err())) return res;
    }

    return Ok();
}

IResult<> BMI160::update(){
    std::array<int16_t, 6> buf;

    if(const auto res = phy_.read_burst(regs_.GYR_ADDRESS, buf);
        unlikely(res.is_err())) return Err(res.unwrap_err());

    regs_.gyr = {buf[0], buf[1], buf[2]};
    regs_.acc = {buf[3], buf[4], buf[5]};

    return Ok();
}

IResult<> BMI160::validate(){
    uint8_t dummy;
    if(const auto res = read_reg(0x7f, dummy);
        unlikely(res.is_err())) return Err(res.unwrap_err());

    auto & reg = regs_.chip_id;
    if(const auto res = read_reg(reg);
        unlikely(res.is_err())) return Err(res.unwrap_err());

    if(reg.data != reg.CORRECT_ID)
        return CHECK_ERR(Err(Error::WrongWhoAmI), "read id is", reg.data);

    return Ok();
}

IResult<> BMI160::reset(){
    return write_command(std::bit_cast<uint8_t>(Command::SOFT_RESET));
}

IResult<Vec3<q24>> BMI160::read_acc(){
    auto conv = [&](const int16_t x) -> real_t{
        return s16_to_uni(x) * acc_scale_;
    };
    
    return Ok{Vec3<q24>{
        conv(regs_.acc.x),
        conv(regs_.acc.y),
        conv(regs_.acc.z)
    }};
}

IResult<Vec3<q24>> BMI160::read_gyr(){
    auto conv = [&](const int16_t x) -> real_t{
        return s16_to_uni(x) * gyr_scale_;
    };
    
    // DEBUG_PRINTLN(regs_.gyr_reg.vec);
    return Ok{Vec3<q24>{
        conv(regs_.gyr.x),
        conv(regs_.gyr.y),
        conv(regs_.gyr.z)
    }};
}

IResult<> BMI160::set_pmu_mode(const PmuType pmu, const PmuMode mode){
    const auto cmd = [&] -> uint8_t {
        switch(pmu){
            case PmuType::ACC:
                return(std::bit_cast<uint8_t>(Command::ACC_SET_PMU) | std::bit_cast<uint8_t>(mode));
            case PmuType::GYR:
                return(std::bit_cast<uint8_t>(Command::GYR_SET_PMU) | std::bit_cast<uint8_t>(mode));
            case PmuType::MAG:
                return(std::bit_cast<uint8_t>(Command::MAG_SET_PMU) | std::bit_cast<uint8_t>(mode));
        }
        __builtin_unreachable();
    }();

    return write_command(cmd);
}

IResult<BMI160::PmuMode> BMI160::get_pmu_mode(const PmuType type){
    auto & reg = regs_.pmu_status;

    if(const auto res = read_reg(reg);
        unlikely(res.is_err())) return Err(res.unwrap_err());

    switch(type){
        case PmuType::ACC:  return  Ok(PmuMode(reg.acc_pmu_status));
        case PmuType::GYR:  return  Ok(PmuMode(reg.gyr_pmu_status));
        case PmuType::MAG:  return  Ok(PmuMode(reg.mag_pmu_status));
    }

    __builtin_unreachable();
}

IResult<> BMI160::set_acc_odr(const AccOdr odr){
    auto reg = RegCopy(regs_.acc_conf);
    reg.acc_odr = odr;
    reg.acc_bwp = 0b010;
    return write_reg(reg);
}

IResult<> BMI160::set_acc_fs(const AccFs fs){
    auto reg = RegCopy(regs_.acc_fs);
    reg.acc_fs = fs;
    if(const auto res = write_reg(reg);
        unlikely(res.is_err())) return res;
    this->acc_scale_ = this->accfs_to_scale(fs);
    return Ok();
}

IResult<> BMI160::set_gyr_odr(const GyrOdr odr){
    auto reg = RegCopy(regs_.gyr_conf);
    reg.gyr_odr = odr;
    reg.gyr_bwp = 0b010;
    return write_reg(reg);

}
IResult<> BMI160::set_gyr_fs(const GyrFs fs){
    auto reg = RegCopy(regs_.gyr_fs);
    reg.gyr_fs = fs;
    if(const auto res = write_reg(reg);
        unlikely(res.is_err())) return res;
    this->gyr_scale_ = this->gyrfs_to_scale(fs);
    return Ok();
}

