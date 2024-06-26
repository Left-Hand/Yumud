#include "mpu6050.hpp"

void MPU6050::init(){
    writeReg(0x6b, 0);
    writeReg(0x19, 0x00);
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
    s16_to_uni(accel.x, x); x *= accel_scaler;
    s16_to_uni(accel.y, y); y *= accel_scaler;
    s16_to_uni(accel.z, z); z *= accel_scaler;
}

void MPU6050::getGyro(real_t &x, real_t &y, real_t &z){
    // x = Reg16ToI16(gyroXReg) * gyro_scaler;
    s16_to_uni(gyro.x, x); x *= gyro_scaler;
    s16_to_uni(gyro.y, y); y *= gyro_scaler;
    s16_to_uni(gyro.z, z); z *= gyro_scaler;
    // uart2.println(gyro.x, gyro.y, gyro.z);
    // x = real_t(gyro.x);
    // y = real_t(gyro.y);
    // z = real_t(gyro.z);
}

void MPU6050::getTemperature(real_t &temp){
    // temp = real_t(36.65f) + Reg16ToI16(temprature) / real_t(340);
}


