#include "mpu6050.hpp"

using namespace ymd::drivers;
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

void MPU6050::update(){
    requestData(RegAddress::AccelX, &accel.x, 7);
}

std::tuple<real_t, real_t, real_t> MPU6050::getAccel(){
    real_t x = s16_to_uni(accel.x) * accel_scaler;
    real_t y = s16_to_uni(accel.y) * accel_scaler;
    real_t z = s16_to_uni(accel.z) * accel_scaler;
    return {x, y, z};
}

std::tuple<real_t, real_t, real_t> MPU6050::getGyro(){
    real_t x = s16_to_uni(gyro.x) * gyro_scaler;
    real_t y = s16_to_uni(gyro.y) * gyro_scaler;
    real_t z = s16_to_uni(gyro.z) * gyro_scaler;
    return {x, y, z};
}

real_t MPU6050::getTemperature(){
    return real_t(36.65f) + s16_to_uni(temprature) / real_t(340);
}


