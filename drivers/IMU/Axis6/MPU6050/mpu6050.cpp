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

void MPU6050::update(){
    requestData(RegAddress::AccelX, &accel.x, 7);
}

std::tuple<real_t, real_t, real_t> MPU6050::getAccel(){
    real_t x, y, z;
    s16_to_uni(accel.x, x); x *= accel_scaler;
    s16_to_uni(accel.y, y); y *= accel_scaler;
    s16_to_uni(accel.z, z); z *= accel_scaler;
    return {x, y, z};
}

std::tuple<real_t, real_t, real_t> MPU6050::getGyro(){
    real_t x, y, z;
    s16_to_uni(gyro.x, x); x *= gyro_scaler;
    s16_to_uni(gyro.y, y); y *= gyro_scaler;
    s16_to_uni(gyro.z, z); z *= gyro_scaler;
    return {x, y, z};
}

void MPU6050::getTemperature(real_t &temp){
    real_t dummy;
    s16_to_uni(temprature, dummy);
    temp = real_t(36.65f) + dummy / real_t(340);
}


