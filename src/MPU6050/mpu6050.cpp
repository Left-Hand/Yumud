#include "mpu6050.hpp"

void MPU6050::init(){
    writeReg(0x6b, 0);
    writeReg(0x19, 0x07);
    writeReg(0x1a, 0x06);
    writeReg(0x13, 0);
    writeReg(0x15, 0);
    writeReg(0x17, 0);
    writeReg(0x38, 0x00);
    writeReg(0x1b, 0x18);
    writeReg(0x1c, 0x00);
}

void MPU6050::getAccel(){
    requestData(Reg::Accel_x, (uint8_t *)(&this->accel.x), 2);
    requestData(Reg::Accel_y, (uint8_t *)(&this->accel.y), 2);
    requestData(Reg::Accel_z, (uint8_t *)(&this->accel.z), 2);
    uart1.println(accel.x, accel.y, accel.z);
}

void MPU6050::getTemprature(){
    requestData(Reg::Temprature, (uint8_t *)(&this->temprature), sizeof(this->temprature));
    // uart1.println(36.53+temprature/340.0);
    // uart1.println(temprature << 8 | temprature >> 8);
    // uart1.println(temprature);
}


