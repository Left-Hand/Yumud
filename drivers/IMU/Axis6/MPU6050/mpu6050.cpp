#include "mpu6050.hpp"
#include "core/debug/debug.hpp"

#define MPU6050_DEBUG_EN

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
#endif


#define RETURN_ON_ERR(x) ({\
    if(const auto __res_return_on_err = (x); __res_return_on_err.is_err()){\
        return CHECK_RES(__res_return_on_err);\
    }\
});\

using namespace ymd;
using namespace ymd::drivers;

using Error = MPU6050::Error;

MPU6050::MPU6050(const hal::I2cDrv i2c_drv, const Package package):
    phy_(i2c_drv),
    package_(package){
    }
    
Result<void, Error> MPU6050::verify(){

    RETURN_ON_ERR(this->reset())

    const auto pkres = this->get_package();
    if(!MPU6050_ASSERT(pkres.is_ok(), "read who am I failed")) 
        return Err(Error(Error::WrongWhoAmI));
    
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
        return Ok();
    }

    return Ok();
}


Result<void, Error> MPU6050::init(){
    RETURN_ON_ERR(verify())
    RETURN_ON_ERR(write_reg(0x6b, 0))
    RETURN_ON_ERR(write_reg(0x19, 0x00))
    RETURN_ON_ERR(write_reg(0x1a, 0x00))
    RETURN_ON_ERR(write_reg(0x13, 0))
    RETURN_ON_ERR(write_reg(0x15, 0))
    RETURN_ON_ERR(write_reg(0x17, 0))
    RETURN_ON_ERR(write_reg(0x38, 0x00))
    RETURN_ON_ERR(set_acc_range(AccRange::_2G))
    RETURN_ON_ERR(set_gyr_range(GyrRange::_1000deg))
    return Ok();
}

Result<void, Error> MPU6050::update(){
    auto res = this->read_burst(acc_x_reg.address, &acc_x_reg, 7);
    data_valid = res.is_ok();
    return res;
}

Option<Vector3_t<real_t>> MPU6050::get_acc(){
    real_t x = uni(acc_x_reg) * acc_scaler_;
    real_t y = uni(acc_y_reg) * acc_scaler_;
    real_t z = uni(acc_z_reg) * acc_scaler_;
    return  Some{Vector3_t<real_t>{x, y, z}};
}

Option<Vector3_t<real_t>> MPU6050::get_gyr(){
    real_t x = uni(gyr_x_reg) * gyr_scaler_;
    real_t y = uni(gyr_y_reg) * gyr_scaler_;
    real_t z = uni(gyr_z_reg) * gyr_scaler_;
    return Some{Vector3_t<real_t>{x, y, z}};
}

Option<real_t> MPU6050::get_temperature(){
    return optcond(data_valid, Some(real_t(36.65f) + uni(temperature_reg) / 340));
}

Result<void, Error> MPU6050::set_acc_range(const AccRange range){
    this->acc_scaler_ = this->calculate_acc_scale(range);

    auto & reg = acc_conf_reg;
    reg.afs_sel = uint8_t(range);
    return this->write_reg(reg);
}

Result<MPU6050::Package, Error> MPU6050::get_package(){
    if(const auto err = read_reg(whoami_reg.address, whoami_reg); err.is_err()){
        MPU6050_PANIC("read who am I failed", err.unwrap_err().as<hal::HalError>().unwrap());
    }
    return Ok{Package(whoami_reg.data)};
}

Result<void, Error> MPU6050::set_gyr_range(const GyrRange range){
    this->gyr_scaler_ = this->calculate_gyr_scale(range);
    auto & reg = gyr_conf_reg;
    reg.fs_sel = uint8_t(range);

    return write_reg(reg);
}


Result<void, Error> MPU6050::reset(){
    const auto berr = phy_.reset();
    if(berr.is_ok()) return Ok();
    else return Err(berr.unwrap_err());
}

Result<void, Error> MPU6050::enable_direct_mode(const Enable en){
    // int_pin_cfg_reg.bypass_en = bool(en);
    int_pin_cfg_reg.as_ref() = 0x22;
    RETURN_ON_ERR(write_reg(int_pin_cfg_reg))
    RETURN_ON_ERR(write_reg(int_pin_cfg_reg))
    RETURN_ON_ERR(write_reg(0x56, 0x01))
    return Ok();
    
}