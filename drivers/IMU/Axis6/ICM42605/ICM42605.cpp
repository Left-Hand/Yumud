#include "ICM42605.hpp"



#ifdef ICM42605_DEBUG_EN
#define ICM42605_TODO(...) TODO()
#define ICM42605_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define ICM42605_PANIC(...) PANIC{__VA_ARGS__}
#define ICM42605_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}


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
#define ICM42605_DEBUG(...)
#define ICM42605_TODO(...) PANIC_NSRC()
#define ICM42605_PANIC(...)  PANIC_NSRC()
#define ICM42605_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
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

// uint8_t ICM42605::GetData(icmData_t *icm, uint8_t MODE) {
//     int16_t out;
//     uint8_t data[6];

//     //加速度计
//     if (MODE & ICM_MODE_ACC) {
//         if (read_regs(ICM42605_ACC_DATA_X1, data, 6) == 0)
//             return 1;
//         out = (int16_t) (data[0] << 8 | data[1]);
//         icm->ax = (float) out * 16 / 32768.0f;
//         out = (int16_t) (data[2] << 8 | data[3]);
//         icm->ay = (float) out * 16 / 32768.0f;
//         out = (int16_t) (data[4] << 8 | data[5]);
//         icm->az = (float) out * 16 / 32768.0f;
//     }

//     //角速度计
//     if (MODE & ICM_MODE_GYR) {
//         if (read_regs(ICM42605_GYR_DATA_X1, data, 6) == 0)
//             return 2;
//         out = (int16_t) (data[0] << 8 | data[1]);
//         icm->gx = (float) out * 2000.0f / 32768.0f;
//         out = (int16_t) (data[2] << 8 | data[3]);
//         icm->gy = (float) out * 2000.0f / 32768.0f;
//         out = (int16_t) (data[4] << 8 | data[5]);
//         icm->gz = (float) out * 2000.0f / 32768.0f;
//     }

//     //温度计
//     if (MODE & ICM_MODE_TEMP) {
//         if (read_regs(ICM42605_TEMP_DATA0, data, 2) == 0)
//             return 3;
//         out = (int16_t) (data[0] << 8 | data[1]);
//         icm->temp = 25.0f + (float) out * 1.0f / 132.48f;
//     }

//     return 0;
// }


IResult<> ICM42605::update(){
    TODO();
    return Ok();
}

IResult<> ICM42605::reset(){
    TODO();
    return Ok();
}

IResult<math::Vec3<iq24>> ICM42605::read_acc(){
    TODO();
    return Ok{math::Vec3<iq24>{0,0,0}};
}

IResult<math::Vec3<iq24>> ICM42605::read_gyr(){
    TODO();
    return Ok{math::Vec3<iq24>{0,0,0}};
}