#ifndef __MPU6050_HPP__

#define __MPU6050_HPP__

#include "i2c/i2cDev.hpp"



class MPU6050{
private:
    BusDrv & busdrv;

public:
    MPU6050(BusDrv & _busdrv):i2cDev(_i2cbus, 0xd0){;}
    void init();
    Vector3 getAccel();
    
    float getTemprature();
        // void test();

};


#endif