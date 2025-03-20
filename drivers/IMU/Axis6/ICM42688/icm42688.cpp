#include "icm42688.hpp"

using namespace ymd;
using namespace ymd::drivers;

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



void ICM42688::init(){
	/*指定Bank0*/
	write_reg(0x76,0x00);
	/*软重启*/
	write_reg(0x11,0x01);
	delay(30);
	/*读取中断位 切换SPI*/
//	buf = IMU->ReadReg(0x2D);
	/*指定Bank0*/
	write_reg(0x76,0x00);
	/*Gyr设置*/
	write_reg(0x4F,0x06);//2000dps 1KHz
	/*Acc设置*/
	write_reg(0x50,0x06);//16G 1KHz
	/*电源管理*/
	write_reg(0x4E,0x0F);//ACC GYR LowNoise Mode
	
	delay(30);

	
	/*指定Bank0*/
	write_reg(0x76,0x00);
	/*中断输出设置*/
//	write_reg(0x14,0x12);//INT1 INT2 脉冲模式，低有效
	/*Gyr设置*/
	write_reg(0x4F,0x06);//2000dps 1KHz
	/*Acc设置*/
	write_reg(0x50,0x06);//16G 1KHz
	/*LSB设置*/
	lsb_acc_x64 = LSB_ACC_16G_x64;
	lsb_gyr_x256 = LSB_GYR_2000_R_x256;
	/*Tem设置&Gyr_Config1*/
	write_reg(0x51,0x56);//BW 82Hz Latency = 2ms
	/*GYR_ACC_CONFIG0*/
	write_reg(0x52,0x11);//1BW
	/*ACC_CONFIG1*/
	write_reg(0x53,0x0D);//Null
	/*INT_CONFIG0*/
	write_reg(0x63,0x00);//Null
	/*INT_CONFIG1*/
//	write_reg(0x64,0x00);//中断引脚正常启用
	/*INT_SOURCE0*/
	write_reg(0x65,0x08);//DRDY INT1
	/*INT_SOURCE1*/
	write_reg(0x66,0x00);//Null
	/*INT_SOURCE3*/
	write_reg(0x68,0x00);//Null
	/*INT_SOURCE3*/
	write_reg(0x69,0x00);//Null
	
/*****抗混叠滤波器@536Hz*****/
	
	/*GYR抗混叠滤波器配置*/
	/*指定Bank1*/
	write_reg(0x76,0x01);
	/*GYR抗混叠滤波器配置*/
	write_reg(0x0B,0xA0);//开启抗混叠和陷波滤波器
	write_reg(0x0C,0x0C);//GYR_AAF_DELT 12 (default 13)
	write_reg(0x0D,0x90);//GYR_AAF_DELTSQR 144 (default 170)
	write_reg(0x0E,0x80);//GYR_AAF_BITSHIFT 8 (default 8)
	
	/*ACC抗混叠滤波器配置*/
	/*指定Bank2*/
	write_reg(0x76,0x02);
	/*ACC抗混叠滤波器配置*/
	write_reg(0x03,0x18);//开启滤波器 ACC_AFF_DELT 12 (default 24)
	write_reg(0x04,0x90);//ACC_AFF_DELTSQR 144 (default 64)
	write_reg(0x05,0x80);//ACC_AAF_BITSHIFT 8 (default 6)

/*****自定义滤波器1号@111Hz*****/

	/*指定Bank0*/
	write_reg(0x76,0x00);
	/*滤波器顺序*/
	write_reg(0x51,0x12);//GYR滤波器1st
	write_reg(0x53,0x05);//ACC滤波器1st
	/*滤波器设置*/
	write_reg(0x52,0x33);//111Hz 03
	/*指定Bank0*/
	write_reg(0x76,0x00);
	/*电源管理*/
	write_reg(0x4E,0x0F);//ACC GYR LowNoise Mode
}

void ICM42688::update(){
	// read_burst(uint8_t(RegAddress::ACC_DATA_X1), &acc_data.x, 3);
	read_burst(uint8_t(RegAddress::ACC_DATA_X1), &acc_data_.x, 6);


	// read_burst(uint8_t(RegAddress::GYR_DATA_X1), &gyr_data.x, 3);
}


bool ICM42688::verify(){
    return false;
}

void ICM42688::reset(){

}

Option<Vector3_t<real_t>> ICM42688::getAcc(){

    auto conv = [this](const real_t x) -> real_t {
        return ((x * this -> lsb_acc_x64) >> 6);
    };

    return Some{Vector3_t<real_t>{
		conv(acc_data_.x), 
		conv(acc_data_.y),
		conv(acc_data_.z)
	}};
}


Option<Vector3_t<real_t>> ICM42688::getGyr(){

    auto conv = [this](const real_t x) -> real_t {
        return ((x * this->lsb_gyr_x256) >> 8);
    };

    return Some{Vector3_t<real_t>{
		conv(gyr_data_.x), 
		conv(gyr_data_.y),
		conv(gyr_data_.z)
	}};
}
