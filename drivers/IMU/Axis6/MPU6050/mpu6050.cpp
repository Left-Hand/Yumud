#include "mpu6050.hpp"
#include "core/debug/debug.hpp"

// #define MPU6050_DEBUG_EN

#ifdef MPU6050_DEBUG_EN
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


#define RETURN_ON_ERR(x) ({\
    if(const auto __res_return_on_err = (x); __res_return_on_err.is_err()){\
        return CHECK_RES(__res_return_on_err);\
    }\
});\

using namespace ymd;
using namespace ymd::drivers;

using Error = MPU6050::Error;

template<typename T = void>
using IResult = Result<T, Error>;

MPU6050::MPU6050(const hal::I2cDrv i2c_drv, const Package package):
    phy_(i2c_drv),
    package_(package){
    }

IResult<> MPU6050::validate(){

    RETURN_ON_ERR(this->reset())

    const auto pkres = this->get_package();
    if(!MPU6050_ASSERT(pkres.is_ok(), "read who am I failed")) 
        return Err(Error::WrongWhoAmI);
    
    const auto package = pkres.unwrap();

    if(package != package_){
        switch(package){
            case Package::MPU6050: MPU6050_DEBUG("this is MPU6050 in fact"); break;
            case Package::MPU6500: MPU6050_DEBUG("this is MPU6500 in fact"); break;
            case Package::MPU9250: MPU6050_DEBUG("this is MPU9250 in fact"); break;
            default: {MPU6050_PANIC(
                "this is unknown device", uint8_t(package)); 
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
    auto res = this->read_burst(
        acc_x_reg.address, std::span(&acc_x_reg.as_ref(), 7));
    is_data_valid_ = res.is_ok();
    return res;
}

IResult<Vector3<q24>> MPU6050::read_acc(){
    real_t x = uni(acc_x_reg.as_val()) * acc_scaler_;
    real_t y = uni(acc_y_reg.as_val()) * acc_scaler_;
    real_t z = uni(acc_z_reg.as_val()) * acc_scaler_;
    return  Ok{Vector3<q24>{x, y, z}};
}

IResult<Vector3<q24>> MPU6050::read_gyr(){
    real_t x = uni(gyr_x_reg.as_val()) * gyr_scaler_;
    real_t y = uni(gyr_y_reg.as_val()) * gyr_scaler_;
    real_t z = uni(gyr_z_reg.as_val()) * gyr_scaler_;
    return Ok{Vector3<q24>{x, y, z}};
}

IResult<real_t> MPU6050::read_temp(){
    static constexpr auto INV_340 = real_t(1.0 / 340);
    return Ok(real_t(36.65f) + uni(temperature_reg.as_val()) * INV_340);
}

IResult<> MPU6050::set_acc_fs(const AccFs range){
    this->acc_scaler_ = this->calculate_acc_scale(range);

    auto reg = RegCopy(acc_conf_reg);
    reg.afs_sel = range;
    return this->write_reg(reg);
}

Result<MPU6050::Package, Error> MPU6050::get_package(){
    if(const auto err = read_reg(whoami_reg); err.is_err()){
        MPU6050_PANIC("read who am I failed", err.unwrap_err().as<hal::HalError>().unwrap());
    }
    return Ok{Package(whoami_reg.data)};
}

IResult<> MPU6050::set_gyr_fs(const GyrFs range){
    this->gyr_scaler_ = this->calculate_gyr_scale(range);
    auto reg = RegCopy(gyr_conf_reg);
    reg.fs_sel = range;

    return write_reg(reg);
}

IResult<> MPU6050::reset(){
    if(const auto res = phy_.reset(); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}
IResult<> MPU6050::enable_direct_mode(const Enable en){
    // int_pin_cfg_reg.bypass_en = bool(en);
    auto reg = RegCopy(int_pin_cfg_reg);
    reg.as_ref() = 0x22;
    RETURN_ON_ERR(write_reg(reg))
    // RETURN_ON_ERR(write_reg(int_pin_cfg_reg))
    RETURN_ON_ERR(write_reg(0x56, 0x01))
    return Ok();
    
}