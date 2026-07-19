#include "ICM42605.hpp"



#ifdef ICM42605_DEBUG_EN
#define ICM42605_TODO(...) TODO()
#define ICM42605_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define ICM42605_PANIC(...) PANIC{__VA_ARGS__}
#define ICM42605_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}




#define RAISE_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    PANIC{#x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define ICM42605_DEBUG(...)
#define ICM42605_TODO(...) PANIC_NSRC()
#define ICM42605_PANIC(...)  PANIC_NSRC()
#define ICM42605_ASSERT(cond, ...) ASSERT_NSRC(cond)


#define RAISE_ERR(x, ...) (x)
#endif

static constexpr uint8_t ICM_MODE_ACC                        = (1<<0);
static constexpr uint8_t ICM_MODE_GYR                        = (1<<1);
static constexpr uint8_t ICM_MODE_TEMP                       = (1<<2);


using namespace ymd;
using namespace ymd::drivers;

using Error = ICM42605::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> ICM42605::init() {
    if(const auto res = write_reg(RegAddr::BANK_SEL, 0);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(RegAddr::BANK_SEL, 1);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(RegAddr::INTF_CONFIG4, 0x02);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(RegAddr::BANK_SEL, 0);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(RegAddr::GYR_CONFIG0, 0b00000110);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(RegAddr::ACC_CONFIG0, 0b00000011);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(RegAddr::PWR_MGMT0, 0b00011111);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> ICM42605::validate(){
    static constexpr uint8_t VALID_WHO_AM_I = 0;
    TODO();

    if(const auto res = write_reg(RegAddr::BANK_SEL, 0);
        res.is_err()) return Err(res.unwrap_err());
    uint8_t id = 0;
    if(const auto res = read_reg(RegAddr::WHO_AM_I, id);
        res.is_err()) return Err(res.unwrap_err());

    if(id != VALID_WHO_AM_I) return Err(Error::InvalidChipId);

    return Ok();
}


IResult<> ICM42605::update(){
    TODO();
    return Ok();
}

IResult<> ICM42605::reset(){
    TODO();
    return Ok();
}

IResult<math::Vec3<iq24>> ICM42605::get_acc(){
    TODO();
    return Ok{math::Vec3<iq24>{0,0,0}};
}

IResult<math::Vec3<iq24>> ICM42605::get_gyr(){
    TODO();
    return Ok{math::Vec3<iq24>{0,0,0}};
}