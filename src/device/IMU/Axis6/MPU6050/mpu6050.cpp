#include "mpu6050.hpp"

void MPU6050::init(){
    writeReg(0x6b, 0);
    writeReg(0x19, 0x07);
    writeReg(0x1a, 0x00);
    writeReg(0x13, 0);
    writeReg(0x15, 0);
    writeReg(0x17, 0);
    writeReg(0x38, 0x00);
    writeReg(0x1b, 0x18);
    writeReg(0x1c, 0x00);
}

void MPU6050::flush(){
    requestData(RegAddress::AccelX, (uint8_t *)&accel.x, 6 + 2 + 6);
}
void MPU6050::getAccel(real_t &x, real_t &y, real_t &z){
    // u16_to_uni(accelXReg, x); x *= real_t(9.8);
    // x = Reg16ToI16(accelXReg) * real_t(9.8)/real_t(16384);
    // y = Reg16ToI16(accelYReg) * real_t(9.8)/real_t(16384);
    // z = Reg16ToI16(accelZReg) * real_t(9.8)/real_t(16384);
}

void MPU6050::getGyro(real_t &x, real_t &y, real_t &z){
    // x = Reg16ToI16(gyroXReg) * gyro_scaler;
    s16_to_uni(gyro.x, x); x *= gyro_scaler;
    s16_to_uni(gyro.y, y); y *= gyro_scaler / 2;
    s16_to_uni(gyro.z, z); z *= gyro_scaler / 2;
        uart2.println(gyro.x, gyro.y, gyro.z);
}

void MPU6050::getTemperature(real_t &temp){
    // temp = real_t(36.65f) + Reg16ToI16(temprature) / real_t(340);
}


