#include "icm42688.hpp"

using namespace ymd;
using namespace ymd::drivers;

#ifdef ICM42688_DEBUG
#undef ICM42688_DEBUG
#define ICM42688_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define ICM42688_DEBUG(...)
#endif

scexpr real_t LSB_ACC_16G_x64 = real_t(64 *  0.0047856934);
scexpr real_t  LSB_ACC_8G_x64 = real_t(64 *  0.0023928467);
scexpr real_t  LSB_ACC_4G_x64 = real_t(64 *  0.0011964233);
scexpr real_t  LSB_ACC_2G_x64 = real_t(64 * 0.00059821167);

/*Turn Into Radian*/
scexpr real_t LSB_GYR_2000_R_x256	= real_t(256 * 0.0010652644);
scexpr real_t LSB_GYR_1000_R_x256	= real_t(256 * 0.00053263222);
scexpr real_t  LSB_GYR_500_R_x256	= real_t(256 * 0.00026631611);
scexpr real_t  LSB_GYR_250_R_x256	= real_t(256 * 0.00013315805);
scexpr real_t LSB_GYR_125D_R_x256  	= real_t(256 * 0.000066579027);	


using Error = ICM42688::Error;

template<typename T = void>
using IResult = Result<T, Error>; 

IResult<> ICM42688::init(){
	/*指定Bank0*/
	if(const auto res = phy_.write_reg(0x76,0x00);
		res.is_err()) return Err(res.unwrap_err());
	/*软重启*/
	if(const auto res = phy_.write_reg(0x11,0x01);
		res.is_err()) return Err(res.unwrap_err());
	delay(30);
	/*读取中断位 切换SPI*/
//	buf = IMU->ReadReg(0x2D);
	/*指定Bank0*/
	if(const auto res = phy_.write_reg(0x76,0x00);
		res.is_err()) return Err(res.unwrap_err());
	/*Gyr设置*/
	if(const auto res = phy_.write_reg(0x4F,0x06);//2000dps 1KHz
		res.is_err()) return Err(res.unwrap_err());
	/*Acc设置*/
	if(const auto res = phy_.write_reg(0x50,0x06);//16G 1KHz
		res.is_err()) return Err(res.unwrap_err());
	/*电源管理*/
	if(const auto res = phy_.write_reg(0x4E,0x0F);//ACC GYR LowNoise Mode
		res.is_err()) return Err(res.unwrap_err());
	
	delay(30);

	
	/*指定Bank0*/
	if(const auto res = phy_.write_reg(0x76,0x00);
		res.is_err()) return Err(res.unwrap_err());
	/*中断输出设置*/
	if(const auto res = phy_.write_reg(0x14,0x12);//INT1 INT2 脉冲模式，低有效
		res.is_err()) return Err(res.unwrap_err());
	/*Gyr设置*/
	if(const auto res = phy_.write_reg(0x4F,0x06);//2000dps 1KHz
		res.is_err()) return Err(res.unwrap_err());
	/*Acc设置*/
	if(const auto res = phy_.write_reg(0x50,0x06);//16G 1KHz
		res.is_err()) return Err(res.unwrap_err());
	/*LSB设置*/
	lsb_acc_x64 = LSB_ACC_16G_x64;
	lsb_gyr_x256 = LSB_GYR_2000_R_x256;
	/*Tem设置&Gyr_Config1*/
	if(const auto res = phy_.write_reg(0x51,0x56);//BW 82Hz Latency = 2ms
		res.is_err()) return Err(res.unwrap_err());
	/*GYR_ACC_CONFIG0*/
	if(const auto res = phy_.write_reg(0x52,0x11);//1BW
		res.is_err()) return Err(res.unwrap_err());
	/*ACC_CONFIG1*/
	if(const auto res = phy_.write_reg(0x53,0x0D);//Null
		res.is_err()) return Err(res.unwrap_err());
	/*INT_CONFIG0*/
	if(const auto res = phy_.write_reg(0x63,0x00);//Null
		res.is_err()) return Err(res.unwrap_err());
	/*INT_CONFIG1*/
	if(const auto res = phy_.write_reg(0x64,0x00);//中断引脚正常启用
		res.is_err()) return Err(res.unwrap_err());

	/*INT_SOURCE0*/
	if(const auto res = phy_.write_reg(0x65,0x08);//DRDY INT1
		res.is_err()) return Err(res.unwrap_err());
	/*INT_SOURCE1*/
	if(const auto res = phy_.write_reg(0x66,0x00);//Null
		res.is_err()) return Err(res.unwrap_err());
	/*INT_SOURCE3*/
	if(const auto res = phy_.write_reg(0x68,0x00);//Null
		res.is_err()) return Err(res.unwrap_err());
	/*INT_SOURCE3*/
	if(const auto res = phy_.write_reg(0x69,0x00);//Null
		res.is_err()) return Err(res.unwrap_err());
	
/*****抗混叠滤波器@536Hz*****/
	
	/*GYR抗混叠滤波器配置*/
	/*指定Bank1*/
	if(const auto res = phy_.write_reg(0x76,0x01);
		res.is_err()) return Err(res.unwrap_err());
	/*GYR抗混叠滤波器配置*/
	if(const auto res = phy_.write_reg(0x0B,0xA0);//开启抗混叠和陷波滤波器
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = phy_.write_reg(0x0C,0x0C);//GYR_AAF_DELT 12 (default 13)
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = phy_.write_reg(0x0D,0x90);//GYR_AAF_DELTSQR 144 (default 170)
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = phy_.write_reg(0x0E,0x80);//GYR_AAF_BITSHIFT 8 (default 8)
		res.is_err()) return Err(res.unwrap_err());
	
	/*ACC抗混叠滤波器配置*/
	/*指定Bank2*/
	if(const auto res = phy_.write_reg(0x76,0x02);
		res.is_err()) return Err(res.unwrap_err());
	/*ACC抗混叠滤波器配置*/
	if(const auto res = phy_.write_reg(0x03,0x18);//开启滤波器 ACC_AFF_DELT 12 (default 24)
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = phy_.write_reg(0x04,0x90);//ACC_AFF_DELTSQR 144 (default 64)
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = phy_.write_reg(0x05,0x80);//ACC_AAF_BITSHIFT 8 (default 6)
		res.is_err()) return Err(res.unwrap_err());

/*****自定义滤波器1号@111Hz*****/

	/*指定Bank0*/
	if(const auto res = phy_.write_reg(0x76,0x00);
		res.is_err()) return Err(res.unwrap_err());
	/*滤波器顺序*/
	if(const auto res = phy_.write_reg(0x51,0x12);//GYR滤波器1st
		res.is_err()) return Err(res.unwrap_err());
	if(const auto res = phy_.write_reg(0x53,0x05);//ACC滤波器1st
		res.is_err()) return Err(res.unwrap_err());
	/*滤波器设置*/
	if(const auto res = phy_.write_reg(0x52,0x33);//111Hz 03
		res.is_err()) return Err(res.unwrap_err());
	/*指定Bank0*/
	if(const auto res = phy_.write_reg(0x76,0x00);
		res.is_err()) return Err(res.unwrap_err());
	/*电源管理*/
	if(const auto res = phy_.write_reg(0x4E,0x0F);//ACC GYR LowNoise Mode
		res.is_err()) return Err(res.unwrap_err());

	return Ok();
}

IResult<>  ICM42688::update(){
	// read_burst(uint8_t(RegAddress::ACC_DATA_X1), &acc_data.x, 3);
	return phy_.read_burst(uint8_t(RegAddress::ACC_DATA_X1), &acc_data_.x, 6);


	// read_burst(uint8_t(RegAddress::GYR_DATA_X1), &gyr_data.x, 3);
}


IResult<>  ICM42688::validate(){
	TODO();
    // return false;
	return Ok();
}

IResult<>  ICM42688::reset(){
	TODO();
	return Ok();
}

Option<Vector3_t<real_t>> ICM42688::get_acc(){

    auto conv = [this](const real_t x) -> real_t {
        return ((x * this -> lsb_acc_x64) >> 6);
    };

    return Some{Vector3_t<real_t>{
		conv(acc_data_.x), 
		conv(acc_data_.y),
		conv(acc_data_.z)
	}};
}


Option<Vector3_t<real_t>> ICM42688::get_gyr(){

    auto conv = [this](const real_t x) -> real_t {
        return ((x * this->lsb_gyr_x256) >> 8);
    };

    return Some{Vector3_t<real_t>{
		conv(gyr_data_.x), 
		conv(gyr_data_.y),
		conv(gyr_data_.z)
	}};
}
