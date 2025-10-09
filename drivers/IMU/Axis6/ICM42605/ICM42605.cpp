#include "ICM42605.hpp"

static constexpr uint8_t ICM42605_DEVICE_CONFIG = 0x11;
static constexpr uint8_t ICM42605_DRIVE_CONFIG = 0x13;
static constexpr uint8_t ICM42605_ACC_DATA_X1 = 0x1F;
static constexpr uint8_t ICM42605_ACC_DATA_X0 = 0x20;
static constexpr uint8_t ICM42605_ACC_DATA_Y1 = 0x21;
static constexpr uint8_t ICM42605_ACC_DATA_Y0 = 0x22;
static constexpr uint8_t ICM42605_ACC_DATA_Z1 = 0x23;
static constexpr uint8_t ICM42605_ACC_DATA_Z0 = 0x24;
static constexpr uint8_t ICM42605_GYR_DATA_X1 = 0x25;
static constexpr uint8_t ICM42605_GYR_DATA_X0 = 0x26;
static constexpr uint8_t ICM42605_GYR_DATA_Y1 = 0x27;
static constexpr uint8_t ICM42605_GYR_DATA_Y0 = 0x28;
static constexpr uint8_t ICM42605_GYR_DATA_Z1 = 0x29;
static constexpr uint8_t ICM42605_GYR_DATA_Z0 = 0x2A;
static constexpr uint8_t ICM42605_TEMP_DATA1 = 0x1D;
static constexpr uint8_t ICM42605_TEMP_DATA0 = 0x1E;
static constexpr uint8_t ICM42605_PWR_MGMT0 = 0x4E;
static constexpr uint8_t ICM42605_GYR_CONFIG0 = 0x4F;
static constexpr uint8_t ICM42605_ACC_CONFIG0 = 0x50;
static constexpr uint8_t ICM42605_GYR_CONFIG1 = 0x51;
static constexpr uint8_t ICM42605_GYR_ACC_CONFIG0 = 0x52;
static constexpr uint8_t ICM42605_ACC_CONFIG1 = 0x53;
static constexpr uint8_t ICM42605_WHO_AM_I = 0x75;
static constexpr uint8_t ICM42605_BANK_SEL = 0x76;
static constexpr uint8_t ICM42605_INTF_CONFIG4 = 0x7A;

static constexpr uint8_t ICM_MODE_ACC                        = (1<<0);
static constexpr uint8_t ICM_MODE_GYR                        = (1<<1);
static constexpr uint8_t ICM_MODE_TEMP                       = (1<<2);


using namespace ymd;
using namespace ymd::drivers;

using Error = ICM42605::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> ICM42605::init() {
    if(const auto res = phy_.write_reg(ICM42605_BANK_SEL, 0);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = phy_.write_reg(ICM42605_BANK_SEL, 1);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = phy_.write_reg(ICM42605_INTF_CONFIG4, 0x02);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = phy_.write_reg(ICM42605_BANK_SEL, 0);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = phy_.write_reg(ICM42605_GYR_CONFIG0, 0b00000110);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = phy_.write_reg(ICM42605_ACC_CONFIG0, 0b00000011);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = phy_.write_reg(ICM42605_PWR_MGMT0, 0b00011111);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> ICM42605::validate(){
    static constexpr uint8_t VALID_WHO_AM_I = 0;
    TODO();

    if(const auto res = phy_.write_reg(ICM42605_BANK_SEL, 0);
        res.is_err()) return Err(res.unwrap_err());
    uint8_t id = 0;
    if(const auto res = phy_.read_reg(ICM42605_WHO_AM_I, id);
        res.is_err()) return Err(res.unwrap_err());

    if(id != VALID_WHO_AM_I) return Err(Error::WrongWhoAmI);

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
    return Ok();
}

IResult<> ICM42605::reset(){
    return Ok();
}

IResult<Vec3<q24>> ICM42605::read_acc(){
    TODO();
    return Ok{Vec3<q24>{0,0,0}};
}

IResult<Vec3<q24>> ICM42605::read_gyr(){
    TODO();
    return Ok{Vec3<q24>{0,0,0}};
}