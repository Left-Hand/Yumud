#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"

namespace ymd::drivers{

class MPU6050:public Axis6{
public:
    scexpr uint8_t default_i2c_addr = 0xd0;

    enum class DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class AccRange:uint8_t{
        _2G     =   0,
        _4G     =   1,
        _8G     =   2,
        _16G    =   3
    };

    enum class GyrRange:uint8_t{
        _250deg     =   0,
        _500deg     =   1,
        _1000deg    =   2,
        _2000deg    =   3
    };

protected:
    I2cDrv i2c_drv_;


    
    #define REG16I_QUICK_DEF(addr, typename, name)\
    struct typename :public Reg16i{scexpr uint8_t address = addr; int16_t :16;} name

    #define REG16_QUICK_DEF(addr, typename, name)\
    struct typename :public Reg16{scexpr uint8_t address = addr; int16_t :16;} name;
    
    #define REG8_QUICK_DEF(addr, typename, name)\
    struct typename :public Reg16i{scexpr uint8_t address = addr; int16_t :16;} name;


    
    struct GyrConfReg:public Reg8{
        scexpr uint8_t address = 0x1b;

        const uint8_t __resv__:3 = 0;
        uint8_t fs_sel:2;
        uint8_t zg_st:1 = 0;
        uint8_t yg_st:1 = 0;
        uint8_t xg_st:1 = 0;
    } gyr_conf_reg;
    

    struct AccConfReg:public Reg8{
        scexpr uint8_t address = 0x1c;

        const uint8_t __resv__:3 = 0;
        uint8_t afs_sel:2;
        uint8_t zg_st:1 = 0;
        uint8_t yg_st:1 = 0;
        uint8_t xg_st:1 = 0;
    } acc_conf_reg;
    
    REG16I_QUICK_DEF(0x3B, AccXReg, acc_x_reg);
    REG16I_QUICK_DEF(0x3D, AccYReg, acc_y_reg);
    REG16I_QUICK_DEF(0x3F, AccZReg, acc_z_reg);

    REG16_QUICK_DEF(0x41, TemperatureReg, temperature_reg);

    REG16I_QUICK_DEF(0x43, GyrXReg, gyr_x_reg);
    REG16I_QUICK_DEF(0x45, GyrYReg, gyr_y_reg);
    REG16I_QUICK_DEF(0x47, GyrZReg, gyr_z_reg);
    

    real_t acc_scaler = 0;
    real_t gyr_scaler = 0;

    enum RegAddress:uint8_t{
        AccX = 0x3b,
        AccY = 0x3d,
        AccZ = 0x3f,
        Temp = 0x41,
        GyrX = 0x43,
        GyrY = 0x45,
        GyrZ = 0x47,
    };

    void writeReg(const uint8_t addr, const uint8_t data){
        i2c_drv_.writeReg((uint8_t)addr, data, MSB);
    }

    void readReg(const uint8_t addr, uint8_t & data){
        i2c_drv_.readReg((uint8_t)addr, data, MSB);
    }

    void requestData(const uint8_t reg_addr, int16_t * datas, const size_t len){
        i2c_drv_.readMulti((uint8_t)reg_addr, datas, len, MSB);
    }
    
    static real_t calculateAccScale(const AccRange range);
    static real_t calculateGyrScale(const GyrRange range);
public:
    MPU6050(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    MPU6050(I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    MPU6050(I2c & bus, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(bus, i2c_addr){;}

    bool verify();
    void init();
    void update();
    std::tuple<real_t, real_t, real_t> getAcc() override;
    std::tuple<real_t, real_t, real_t> getGyr() override;
    real_t getTemperature();

    void setAccRange(const AccRange range);
    void setGyrRange(const GyrRange range);
};


};