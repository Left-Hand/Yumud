#include "icm42688.hpp"

using namespace yumud::drivers;

scexpr real_t LSB_ACC_16G_x64 = 64 *  0.0047856934_r;
scexpr real_t  LSB_ACC_8G_x64 = 64 *  0.0023928467_r;
scexpr real_t  LSB_ACC_4G_x64 = 64 *  0.0011964233_r;
scexpr real_t  LSB_ACC_2G_x64 = 64 * 0.00059821167_r;

/*Turn Into Radian*/
scexpr real_t LSB_GYRO_2000_R_x256	 = 256 * 0.0010652644_r;
scexpr real_t LSB_GYRO_1000_R_x256	= 256 * 0.00053263222_r;
scexpr real_t  LSB_GYRO_500_R_x256	= 256 * 0.00026631611_r;
scexpr real_t  LSB_GYRO_250_R_x256	= 256 * 0.00013315805_r;
scexpr real_t LSB_GYRO_125D_R_x256  = 256 * 0.000066579027_r;	



void ICM42688::init(){
	/*指定Bank0*/
	writeReg(0x76,0x00);
	/*软重启*/
	writeReg(0x11,0x01);
	delay(30);
	/*读取中断位 切换SPI*/
//	buf = IMU->ReadReg(0x2D);
	/*指定Bank0*/
	writeReg(0x76,0x00);
	/*Gyro设置*/
	writeReg(0x4F,0x06);//2000dps 1KHz
	/*Accel设置*/
	writeReg(0x50,0x06);//16G 1KHz
	/*电源管理*/
	writeReg(0x4E,0x0F);//ACC GYRO LowNoise Mode
	
	delay(30);

	
	/*指定Bank0*/
	writeReg(0x76,0x00);
	/*中断输出设置*/
//	writeReg(0x14,0x12);//INT1 INT2 脉冲模式，低有效
	/*Gyro设置*/
	writeReg(0x4F,0x06);//2000dps 1KHz
	/*Accel设置*/
	writeReg(0x50,0x06);//16G 1KHz
	/*LSB设置*/
	lsb_acc_x64 = LSB_ACC_16G_x64;
	lsb_gyro_x256 = LSB_GYRO_2000_R_x256;
	/*Tem设置&Gyro_Config1*/
	writeReg(0x51,0x56);//BW 82Hz Latency = 2ms
	/*GYRO_ACCEL_CONFIG0*/
	writeReg(0x52,0x11);//1BW
	/*ACCEL_CONFIG1*/
	writeReg(0x53,0x0D);//Null
	/*INT_CONFIG0*/
	writeReg(0x63,0x00);//Null
	/*INT_CONFIG1*/
//	writeReg(0x64,0x00);//中断引脚正常启用
	/*INT_SOURCE0*/
	writeReg(0x65,0x08);//DRDY INT1
	/*INT_SOURCE1*/
	writeReg(0x66,0x00);//Null
	/*INT_SOURCE3*/
	writeReg(0x68,0x00);//Null
	/*INT_SOURCE3*/
	writeReg(0x69,0x00);//Null
	
/*****抗混叠滤波器@536Hz*****/
	
	/*GYRO抗混叠滤波器配置*/
	/*指定Bank1*/
	writeReg(0x76,0x01);
	/*GYRO抗混叠滤波器配置*/
	writeReg(0x0B,0xA0);//开启抗混叠和陷波滤波器
	writeReg(0x0C,0x0C);//GYRO_AAF_DELT 12 (default 13)
	writeReg(0x0D,0x90);//GYRO_AAF_DELTSQR 144 (default 170)
	writeReg(0x0E,0x80);//GYRO_AAF_BITSHIFT 8 (default 8)
	
	/*ACCEL抗混叠滤波器配置*/
	/*指定Bank2*/
	writeReg(0x76,0x02);
	/*ACCEL抗混叠滤波器配置*/
	writeReg(0x03,0x18);//开启滤波器 ACCEL_AFF_DELT 12 (default 24)
	writeReg(0x04,0x90);//ACCEL_AFF_DELTSQR 144 (default 64)
	writeReg(0x05,0x80);//ACCEL_AAF_BITSHIFT 8 (default 6)

/*****自定义滤波器1号@111Hz*****/

	/*指定Bank0*/
	writeReg(0x76,0x00);
	/*滤波器顺序*/
	writeReg(0x51,0x12);//GYRO滤波器1st
	writeReg(0x53,0x05);//ACCEL滤波器1st
	/*滤波器设置*/
	writeReg(0x52,0x33);//111Hz 03
	/*指定Bank0*/
	writeReg(0x76,0x00);
	/*电源管理*/
	writeReg(0x4E,0x0F);//ACC GYRO LowNoise Mode
}

void ICM42688::update(){
	// requestData(uint8_t(RegAddress::ACCEL_DATA_X1), &accel_data.x, 3);
	requestData(uint8_t(RegAddress::ACCEL_DATA_X1), &accel_data.x, 6);


	// requestData(uint8_t(RegAddress::GYRO_DATA_X1), &gyro_data.x, 3);
}


bool ICM42688::verify(){
    return false;
}

void ICM42688::reset(){

}

std::tuple<real_t, real_t, real_t> ICM42688::getAccel(){

    auto conv = [this](const real_t x) -> real_t {
        return ((x * this -> lsb_acc_x64) >> 6);
    };

    return {conv(accel_data.x), conv(accel_data.y),conv(accel_data.z)};
}


std::tuple<real_t, real_t, real_t> ICM42688::getGyro(){

    auto conv = [this](const real_t x) -> real_t {
        return ((x * this->lsb_gyro_x256) >> 8);
    };

    return {conv(gyro_data.x), conv(gyro_data.y),conv(gyro_data.z)};
}

void ICM42688::writeReg(const uint8_t addr, const uint8_t data){
	if(i2c_drv){
		i2c_drv->writeReg(addr, data, MSB);
	}else if(spi_drv){
		spi_drv->writeSingle(uint8_t(addr), CONT);
		spi_drv->writeSingle(data);

		ICM42688_DEBUG("Wspi", addr, data);
	}else{
		PANIC("not supported driver");
	}
}

void ICM42688::readReg(const uint8_t addr, uint8_t & data){
	if(i2c_drv){
		i2c_drv->readReg((uint8_t)addr, data, MSB);
	}else if(spi_drv){
		spi_drv->writeSingle(uint8_t(uint8_t(addr) | 0x80), CONT);
		spi_drv->readSingle(data);

		ICM42688_DEBUG("Rspi", addr, data);
	}else{

	}
}


void ICM42688::requestData(const uint8_t addr, int16_t * datas, const size_t len){
	if(i2c_drv){
		i2c_drv->readPool(uint8_t(addr), datas, len, MSB);
	}if(spi_drv){
		spi_drv->writeSingle(uint8_t(uint8_t(addr) | 0x80), CONT);
		spi_drv->readMulti((datas), len);

		ICM42688_DEBUG("Rspi", addr, len);
	}else{
		
	}

}
