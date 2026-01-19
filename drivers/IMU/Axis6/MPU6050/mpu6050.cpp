#include "mpu6050.hpp"
#include "core/debug/debug.hpp"

#define MPU6050_DEBUG_EN 0

#if MPU6050_DEBUG_EN == 1
#define MPU6050_TODO(...) TODO()
#define MPU6050_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define MPU6050_PANIC(...) PANIC{__VA_ARGS__}
#define MPU6050_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}


#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    ASSERT{false, #x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define MPU6050_DEBUG(...)
#define MPU6050_TODO(...) PANIC_NSRC()
#define MPU6050_PANIC(...)  PANIC_NSRC()
#define MPU6050_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif


using namespace ymd;
using namespace ymd::drivers;

using Error = MPU6050::Error;

template<typename T = void>
using IResult = Result<T, Error>;

MPU6050::MPU6050(const hal::I2cDrv i2c_drv, const Package package):
    transport_(i2c_drv),
    package_(package)
{}

IResult<> MPU6050::validate(){

    if(const auto res = reset();
        res.is_err()) return Err(res.unwrap_err());

    const auto res = get_package();
    if(!MPU6050_ASSERT(res.is_ok(), "read who am I failed")) 
        return Err(Error::InvalidChipId);
    
    const auto founded_package = res.unwrap();

    if(founded_package != package_){
        switch(founded_package){
            case Package::MPU6050: MPU6050_DEBUG("this is MPU6050 in fact"); break;
            case Package::MPU6500: MPU6050_DEBUG("this is MPU6500 in fact"); break;
            case Package::MPU9250: MPU6050_DEBUG("this is MPU9250 in fact"); break;
            default: {
                MPU6050_PANIC("this is unknown device", uint8_t(founded_package)); 
                return Err(Error(Error::UnknownDevice));
            }
        }
    }

    return Ok();
}

IResult<> MPU6050::init(const Config & cfg){
    if(const auto res = validate();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(0x6b, 0);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(0x19, 0x00);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(0x1a, 0x00);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(0x13, 0);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(0x15, 0);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(0x17, 0);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(0x38, 0x00);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_acc_fs(cfg.acc_fs);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_gyr_fs(cfg.gyr_fs);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> MPU6050::update(){
    auto res = read_burst(
        regs_.acc_x_reg.ADDRESS, std::span(
            reinterpret_cast<int16_t *>(&regs_.acc_x_reg.as_bits_mut()), 
        7));
    is_data_valid_ = res.is_ok();
    return res;
}

IResult<Vec3<iq24>> MPU6050::read_acc(){

    return  Ok{Vec3<iq24>{
        iq16::from_bits(regs_.acc_x_reg.to_bits()) * acc_scaler_,
        iq16::from_bits(regs_.acc_y_reg.to_bits()) * acc_scaler_,
        iq16::from_bits(regs_.acc_z_reg.to_bits()) * acc_scaler_
    }};
}

IResult<Vec3<iq24>> MPU6050::read_gyr(){

    return Ok{Vec3<iq24>{
        iq16::from_bits(regs_.gyr_x_reg.to_bits()) * gyr_scaler_,
        iq16::from_bits(regs_.gyr_y_reg.to_bits()) * gyr_scaler_,
        iq16::from_bits(regs_.gyr_z_reg.to_bits()) * gyr_scaler_
    }};
}

IResult<iq16> MPU6050::read_temp(){
    auto & reg = regs_.temperature_reg;
    static constexpr auto INV_340 = iq16(1.0 / 340);
    return Ok(iq16(36.65f) + iq16::from_bits(reg.to_bits()) * INV_340);
}

IResult<> MPU6050::set_acc_fs(const AccFs fs){
    acc_scaler_ = calculate_acc_scaler(fs);

    auto reg = RegCopy(regs_.acc_conf_reg);
    reg.afs_sel = fs;
    return write_reg(reg);
}

Result<MPU6050::Package, Error> MPU6050::get_package(){
    auto & reg = regs_.whoami_reg;
    if(const auto res = read_reg(reg); 
        res.is_err()) MPU6050_PANIC("read who am I failed", res.unwrap_err());

    return Ok{reg.package};
}

IResult<> MPU6050::set_gyr_fs(const GyrFs fs){
    gyr_scaler_ = calculate_gyr_scaler(fs);
    auto reg = RegCopy(regs_.gyr_conf_reg);
    reg.fs_sel = fs;

    return write_reg(reg);
}

IResult<> MPU6050::reset(){
    if(const auto res = transport_.reset(); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> MPU6050::enable_direct_mode(const Enable en){
    auto reg = RegCopy(regs_.int_pin_cfg_reg);
    reg.as_bits_mut() = 0x22;
    if(const auto res = write_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(0x56, 0x01);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
    
}