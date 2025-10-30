#include "icm42688.hpp"
#include "core/debug/debug.hpp"

#define ICM42688_DEBUG_EN

#ifdef ICM42688_DEBUG_EN

#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(e, ...) ({\
    const auto && __err_check_err = (e);\
    PANIC{e.unwrap(), ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif

static constexpr uint8_t ACC_DATA_X0L_ADDR             = 0x20;
static constexpr uint8_t ACC_DATA_Y0L_ADDR             = 0x22;
static constexpr uint8_t ACC_DATA_Z0L_ADDR             = 0x24;
static constexpr uint8_t GYR_DATA_X0L_ADDR              = 0x26;
static constexpr uint8_t GYR_DATA_Y0L_ADDR              = 0x28;
static constexpr uint8_t GYR_DATA_Z0L_ADDR              = 0x2A;


using namespace ymd;
using namespace ymd::drivers;


using Error = ICM42688::Error;


template<typename T = void>
using IResult = Result<T, Error>; 

IResult<> ICM42688::init(const Config & cfg){
	static constexpr size_t MAX_RETRY_TIMES = 20;
	if(const auto res = retry(MAX_RETRY_TIMES, [&]{return validate();});
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));

	/*软重启*/
	if(const auto res = reset();
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));

	clock::delay(30ms);

	/*Gyr设置*/
	if(const auto res = set_gyr_fs(cfg.gyr_fs);
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));

	if(const auto res = set_gyr_odr(cfg.gyr_odr);
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));

	/*Acc设置*/

	if(const auto res = set_acc_fs(cfg.acc_fs);
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	
	if(const auto res = set_acc_odr(cfg.acc_odr);
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));

	/*电源管理*/
	if(const auto res = write_reg(0x4E,0x0F);//ACC GYR LowNoise Mode
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	
	clock::delay(30ms);

	
	/*指定Bank0*/
	if(const auto res = write_reg(0x76,0x00);
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*中断输出设置*/
	if(const auto res = write_reg(0x14,0x12);//INT1 INT2 脉冲模式，低有效
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));

	/*Tem设置&Gyr_Config1*/
	if(const auto res = write_reg(0x51,0x56);//BW 82Hz Latency = 2ms
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*GYR_ACC_CONFIG0*/
	if(const auto res = write_reg(0x52,0x11);//1BW
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*ACC_CONFIG1*/
	if(const auto res = write_reg(0x53,0x0D);//Null
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*INT_CONFIG0*/
	if(const auto res = write_reg(0x63,0x00);//Null
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*INT_CONFIG1*/
	if(const auto res = write_reg(0x64,0x00);//中断引脚正常启用
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));

	/*INT_SOURCE0*/
	if(const auto res = write_reg(0x65,0x08);//DRDY INT1
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*INT_SOURCE1*/
	if(const auto res = write_reg(0x66,0x00);//Null
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*INT_SOURCE3*/
	if(const auto res = write_reg(0x68,0x00);//Null
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*INT_SOURCE3*/
	if(const auto res = write_reg(0x69,0x00);//Null
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	
	/*****抗混叠滤波器@536Hz*****/
	
	/*GYR抗混叠滤波器配置*/
	/*指定Bank1*/
	if(const auto res = write_reg(0x76,0x01);
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*GYR抗混叠滤波器配置*/
	if(const auto res = write_reg(0x0B,0xA0);//开启抗混叠和陷波滤波器
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	if(const auto res = write_reg(0x0C,0x0C);//GYR_AAF_DELT 12 (default 13)
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	if(const auto res = write_reg(0x0D,0x90);//GYR_AAF_DELTSQR 144 (default 170)
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	if(const auto res = write_reg(0x0E,0x80);//GYR_AAF_BITSHIFT 8 (default 8)
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	
	/*ACC抗混叠滤波器配置*/
	/*指定Bank2*/
	if(const auto res = write_reg(0x76,0x02);
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*ACC抗混叠滤波器配置*/
	if(const auto res = write_reg(0x03,0x18);//开启滤波器 ACC_AFF_DELT 12 (default 24)
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	if(const auto res = write_reg(0x04,0x90);//ACC_AFF_DELTSQR 144 (default 64)
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	if(const auto res = write_reg(0x05,0x80);//ACC_AAF_BITSHIFT 8 (default 6)
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));

	/*****自定义滤波器1号@111Hz*****/

	/*指定Bank0*/
	if(const auto res = write_reg(0x76,0x00);
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*滤波器顺序*/
	if(const auto res = write_reg(0x51,0x12);//GYR滤波器1st
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	if(const auto res = write_reg(0x53,0x05);//ACC滤波器1st
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*滤波器设置*/
	if(const auto res = write_reg(0x52,0x33);//111Hz 03
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*指定Bank0*/
	if(const auto res = write_reg(0x76,0x00);
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
	/*电源管理*/
	if(const auto res = write_reg(0x4E,0x0F);//ACC GYR LowNoise Mode
		res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));

	return Ok();
}

IResult<> ICM42688::set_gyr_odr(const GyrOdr odr){
	auto reg = RegCopy(regs_.gyro_config0_reg);
	reg.gyro_odr = odr;
	return write_reg(reg);
}
IResult<> ICM42688::set_gyr_fs(const GyrFs fs){
	auto reg = RegCopy(regs_.gyro_config0_reg);
	reg.gyro_fs = fs;
	gyr_scale_ = calc_gyr_scale(fs);
	return write_reg(reg);
}

IResult<> ICM42688::set_acc_odr(const AccOdr odr){
	auto reg = RegCopy(regs_.accel_config0_reg);
	reg.accel_odr = odr;
	return write_reg(reg);
}

IResult<> ICM42688::set_acc_fs(const AccFs fs){
	auto reg = RegCopy(regs_.accel_config0_reg);
	reg.accel_fs = fs;
	acc_scale_ = calc_acc_scale(fs);
	return write_reg(reg);
}

IResult<> ICM42688::reset(){
	auto reg = RegCopy(regs_.device_config_reg);
	reg.soft_reset_config = 1;
	if(const auto res = write_reg(reg);
		res.is_err()) return Err(res.unwrap_err());
	reg.soft_reset_config = 0;
	reg.apply();
	return Ok();
}

IResult<>  ICM42688::update(){
	int16_t buf[6];

	if(const auto res = phy_.read_burst(ACC_DATA_X0L_ADDR - 1, std::span(buf, 6));
		res.is_err()) return Err(res.unwrap_err());
	
	regs_.acc_data_ = {buf[0], buf[1], buf[2]};
	regs_.gyr_data_ = {buf[3], buf[4], buf[5]};

	return Ok();
}


IResult<>  ICM42688::validate(){
	auto & reg = regs_.who_am_i_reg;
	if(const auto res = read_reg(reg);
		res.is_err()) return res;
	
	if(reg.data != reg.KEY){
		return CHECK_ERR(Err(Error::InvalidChipId), reg.data);
	}

	return Ok();
}

IResult<Vec3<iq24>> ICM42688::read_acc(){
    return Ok{Vec3<iq24>{
		acc_scale_ * iq24(iq16(regs_.acc_data_.x) >> 16), 
		acc_scale_ * iq24(iq16(regs_.acc_data_.y) >> 16), 
		acc_scale_ * iq24(iq16(regs_.acc_data_.z) >> 16), 
	}};
}


IResult<Vec3<iq24>> ICM42688::read_gyr(){

    return Ok{Vec3<iq24>{
		gyr_scale_ * iq24(iq16(regs_.gyr_data_.x) >> 16),
		gyr_scale_ * iq24(iq16(regs_.gyr_data_.y) >> 16),
		gyr_scale_ * iq24(iq16(regs_.gyr_data_.z) >> 16)
	}};
}
