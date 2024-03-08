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

void MPU6050::flush(){
    requestData(RegAddress::AccelX, accelXReg, 6 + 2 + 6);
}
void MPU6050::getAccel(real_t &x, real_t &y, real_t &z){
    x = Reg16ToI16(accelXReg) * accel_scaler;
    y = Reg16ToI16(accelYReg) * accel_scaler;
    z = Reg16ToI16(accelZReg) * accel_scaler;
}

void MPU6050::getGyro(real_t &x, real_t &y, real_t &z){
    x = Reg16ToI16(gyroXReg) * gyro_scaler;
    y = Reg16ToI16(gyroYReg) * gyro_scaler;
    z = Reg16ToI16(gyroZReg) * gyro_scaler;
}

void MPU6050::getTemperature(real_t &temp){
    temp = real_t(36.65f) + Reg16ToI16(tempReg) / real_t(340);
}


