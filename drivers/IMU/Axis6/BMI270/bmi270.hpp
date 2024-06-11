#ifndef __BMI270_HPP__
#define __BMI270_HPP__

#include "drivers/IMU/IMU.hpp"
#include "src/bus/spi/spidrv.hpp"
#include "types/real.hpp"
#include "src/platform.h"

#define BMI270_DEBUG


#ifdef BMI270_DEBUG
#undef BMI270_DEBUG
#define BMI270_DEBUG(...) DEBUG_LOG("BMI270: ", ##__VA_ARGS__)
#else
#define BMI270_DEBUG(...)
#endif

constexpr uint8_t IMU660RA_DEV_ADDR  =         ( 0x69 )  ;                                  // SA0接地：0x68 SA0上拉：0x69 模块默认上拉
constexpr uint8_t IMU660RA_SPI_W      =        ( 0x00 );
constexpr uint8_t IMU660RA_SPI_R            =  ( 0x80 );


typedef enum
{
    IMU660RA_ACC_SAMPLE_SGN_2G ,                                                // 加速度计量程 ±2G  (ACC = Accelerometer 加速度计) (SGN = signum 带符号数 表示正负范围) (G = g 重力加速度 g≈9.80 m/s^2)
    IMU660RA_ACC_SAMPLE_SGN_4G ,                                                // 加速度计量程 ±4G  (ACC = Accelerometer 加速度计) (SGN = signum 带符号数 表示正负范围) (G = g 重力加速度 g≈9.80 m/s^2)
    IMU660RA_ACC_SAMPLE_SGN_8G ,                                                // 加速度计量程 ±8G  (ACC = Accelerometer 加速度计) (SGN = signum 带符号数 表示正负范围) (G = g 重力加速度 g≈9.80 m/s^2)
    IMU660RA_ACC_SAMPLE_SGN_16G,                                                // 加速度计量程 ±16G (ACC = Accelerometer 加速度计) (SGN = signum 带符号数 表示正负范围) (G = g 重力加速度 g≈9.80 m/s^2)
}imu660ra_acc_sample_config;

typedef enum
{
    IMU660RA_GYRO_SAMPLE_SGN_125DPS ,                                           // 陀螺仪量程 ±125DPS  (GYRO = Gyroscope 陀螺仪) (SGN = signum 带符号数 表示正负范围) (DPS = Degree Per Second 角速度单位 °/S)
    IMU660RA_GYRO_SAMPLE_SGN_250DPS ,                                           // 陀螺仪量程 ±250DPS  (GYRO = Gyroscope 陀螺仪) (SGN = signum 带符号数 表示正负范围) (DPS = Degree Per Second 角速度单位 °/S)
    IMU660RA_GYRO_SAMPLE_SGN_500DPS ,                                           // 陀螺仪量程 ±500DPS  (GYRO = Gyroscope 陀螺仪) (SGN = signum 带符号数 表示正负范围) (DPS = Degree Per Second 角速度单位 °/S)
    IMU660RA_GYRO_SAMPLE_SGN_1000DPS,                                           // 陀螺仪量程 ±1000DPS (GYRO = Gyroscope 陀螺仪) (SGN = signum 带符号数 表示正负范围) (DPS = Degree Per Second 角速度单位 °/S)
    IMU660RA_GYRO_SAMPLE_SGN_2000DPS,                                           // 陀螺仪量程 ±2000DPS (GYRO = Gyroscope 陀螺仪) (SGN = signum 带符号数 表示正负范围) (DPS = Degree Per Second 角速度单位 °/S)
}imu660ra_gyro_sample_config;

#define IMU660RA_ACC_SAMPLE_DEFAULT     ( IMU660RA_ACC_SAMPLE_SGN_8G )          // 在这设置默认的 加速度计 初始化量程
#define IMU660RA_GYRO_SAMPLE_DEFAULT    ( IMU660RA_GYRO_SAMPLE_SGN_2000DPS )    // 在这设置默认的 陀螺仪   初始化量程

#define IMU660RA_TIMEOUT_COUNT      ( 0x00FF )                                  // IMU660RA 超时计数

#define IMU660RA_CHIP_ID            ( 0x00 )
#define IMU660RA_PWR_CONF           ( 0x7C )
#define IMU660RA_PWR_CTRL           ( 0x7D )
#define IMU660RA_INIT_CTRL          ( 0x59 )
#define IMU660RA_INIT_DATA          ( 0x5E )
#define IMU660RA_INT_STA            ( 0x21 )
#define IMU660RA_ACC_ADDRESS        ( 0x0C )
#define IMU660RA_GYRO_ADDRESS       ( 0x12 )
#define IMU660RA_ACC_CONF           ( 0x40 )
#define IMU660RA_ACC_RANGE          ( 0x41 )
#define IMU660RA_GYR_CONF           ( 0x42 )
#define IMU660RA_GYR_RANGE          ( 0x43 )

class BMI270: public Axis6{
public:

    enum class AccelRange:uint8_t{
        G2, G4, G8, G16
    };

    enum class GyroRange:uint8_t{
        DPS125, DPS250, DPS500, DPS1000, DPS2000
    };

protected:
    SpiDrv & bus_drv;
    real_t accel_scaler;
    real_t gyro_scaler;

    #include "bmi270_file.h"

    struct vec3{
        int16_t x;
        int16_t y;
        int16_t z;
    };

    struct{
        vec3 accel;
        vec3 gyro;
    };

    static constexpr uint8_t valid_chipid = 0x24;
    void writeReg(const uint8_t & reg_addr, const uint8_t data){
        bus_drv.write({reg_addr, data});
    }

    void writeRegs(const uint8_t & reg_addr, const uint8_t * datas, const size_t & data_len){
        bus_drv.write(reg_addr, false);
        bus_drv.write(datas, data_len);
    }

    void readReg(const uint8_t & reg_addr, uint8_t & data){
        bus_drv.write((uint8_t)(reg_addr | 0x80), false);
        uint8_t dummy = 0;
        bus_drv.write(dummy, false);
        bus_drv.read(data);
    }

    void readRegs(const uint8_t & reg_addr, uint8_t * datas, const uint8_t & data_len){
        bus_drv.write((uint8_t)(reg_addr | 0x80), false);
        uint8_t dummy = 0;
        bus_drv.write(dummy, false);
        bus_drv.read(datas, data_len);
    }
public:
    BMI270(SpiDrv & _bus_drv):bus_drv(_bus_drv){;}

    void init();

    uint8_t getChipId(){
        uint8_t chip_id;
        readReg(IMU660RA_CHIP_ID, chip_id);
        return chip_id;
    }

    void flush() override{
        readRegs(0x0c, (uint8_t *)&accel.x, 12);
    }

    void getAccel(real_t &x, real_t &y, real_t &z){
        u16_to_uni(accel.x, x); x *= accel_scaler;
        u16_to_uni(accel.y, y); y *= accel_scaler;
        u16_to_uni(accel.z, z); z *= accel_scaler;
    }

    void getGyro(real_t &x, real_t &y, real_t &z){
        u16_to_uni(gyro.x, x); x *= gyro_scaler;
        u16_to_uni(gyro.y, y); y *= gyro_scaler;
        u16_to_uni(gyro.z, z); z *= gyro_scaler;
        uart2.println(gyro.x, gyro.y, gyro.z);
    }

};

#endif