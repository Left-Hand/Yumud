#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"

#include "sys/utils/rustlike/Optional.hpp"
#include "sys/utils/rustlike/Result.hpp"

namespace ymd::drivers{

class MPU6050:public Axis6{
public:
    using DeviceResult = Result<void, BusError>;
    __inline DeviceResult make_result(const BusError res){
        if(res.ok()) return Ok();
        else return Err(res); 
    }


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
    hal::I2cDrv i2c_drv_;


    
    #define REG16I_QUICK_DEF(addr, typename, name)\
    struct typename :public Reg16i<>{scexpr uint8_t address = addr; int16_t :16;} name

    #define REG16_QUICK_DEF(addr, typename, name)\
    struct typename :public Reg16<>{scexpr uint8_t address = addr; int16_t :16;} name;
    
    #define REG8_QUICK_DEF(addr, typename, name)\
    struct typename :public Reg16i<>{scexpr uint8_t address = addr; int16_t :16;} name;


    
    struct GyrConfReg:public Reg8<>{
        scexpr uint8_t address = 0x1b;

        const uint8_t __resv__:3 = 0;
        uint8_t fs_sel:2;
        uint8_t zg_st:1 = 0;
        uint8_t yg_st:1 = 0;
        uint8_t xg_st:1 = 0;
    } gyr_conf_reg;
    

    struct AccConfReg:public Reg8<>{
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

    bool data_valid = false;

    enum RegAddress:uint8_t{
        AccX = 0x3b,
        AccY = 0x3d,
        AccZ = 0x3f,
        Temp = 0x41,
        GyrX = 0x43,
        GyrY = 0x45,
        GyrZ = 0x47,
    };

    [[nodiscard]] DeviceResult writeReg(const uint8_t addr, const uint8_t data);

    [[nodiscard]] DeviceResult readReg(const uint8_t addr, uint8_t & data);

    [[nodiscard]] DeviceResult requestData(const uint8_t reg_addr, int16_t * datas, const size_t len);
    
    static constexpr real_t calculateAccScale(const AccRange range){
        constexpr double g = 9.806;
        switch(range){
            default:
            case AccRange::_2G:
                return real_t(g * 2);
            case AccRange::_4G:
                return real_t(g * 4);
            case AccRange::_8G:
                return real_t(g * 8);
            case AccRange::_16G:
                return real_t(g * 16);
        }
    }

    static constexpr real_t calculateGyrScale(const GyrRange range){
        switch(range){
            default:
            case GyrRange::_250deg:
                return real_t(ANGLE2RAD(250));
            case GyrRange::_500deg:
                return real_t(ANGLE2RAD(500));
            case GyrRange::_1000deg:
                return real_t(ANGLE2RAD(1000));
            case GyrRange::_2000deg:
                return real_t(ANGLE2RAD(2000));
        }
    }
public:
    MPU6050(const MPU6050 & other) = delete;
    MPU6050(MPU6050 && other) = delete;

    MPU6050(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    MPU6050(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    MPU6050(hal::I2c & bus, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(bus, i2c_addr){;}

    bool verify();

    void init();
    
    void update();

    [[nodiscard]] std::tuple<real_t, real_t, real_t> getAcc() override;
    [[nodiscard]] std::tuple<real_t, real_t, real_t> getGyr() override;

    [[nodiscard]] Option<std::tuple<real_t, real_t, real_t>> getAcc2(){
        return Some{getAcc()};
    }

    [[nodiscard]] Option<std::tuple<real_t, real_t, real_t>> getGyr2(){
        return Some{getGyr()};
    }


    [[nodiscard]] Option<real_t> getTemperature();

    void setAccRange(const AccRange range);
    void setGyrRange(const GyrRange range);
};


};