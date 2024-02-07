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
    requestData(RegAddress::Accel_x, (uint8_t *)&accel, 6 + 2 + 6);
    // uart1.println(accel.x, accel.y, accel.z, 36.53+temperature/340.0, gyro.x, gyro.y, gyro.z);
}
void MPU6050::getAccel(){
    // uint8_t buf[6] = {0};
    // accel.x = ((uint16_t)buf[1] << 8) | buf[0];
    // accel.y = buf[2] << 8 | buf[3];
    // accel.z = buf[4] << 8 | buf[5];

}

void MPU6050::getTemperature(){
    requestData(RegAddress::Temperature, (uint8_t *)(&this -> temperature), sizeof(this -> temperature));
    // uart1.println(36.53+temperature/340.0);
    // uart1.println(temperature << 8 | temperature >> 8);
    // uart1.println(temperature);
}


