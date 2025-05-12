#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include <tuple>

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{

class QMC5883L:public MagnetometerIntf{
public:
    enum class DataRate:uint8_t{
        DR10, DR50, DR100, DR200
    };

    enum class OverSampleRatio:uint8_t{
        OSR512, OSR256, OSR128, OSR64
    };

    enum class FullScale:uint8_t{
        FS2G, FS8G
    };

    enum class Mode:uint8_t{
        Single,Continuous
    };

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x1a);
protected:
    hal::I2cDrv i2c_drv_;

    real_t fs;
    uint8_t ovsfix = 0;

    struct MagXReg:public Reg16i<>{
        int16_t :16;
    };

    struct MagYReg:public Reg16i<>{
        int16_t :16;
    };

    struct MagZReg:public Reg16i<>{
        int16_t :16;
    };

    struct StatusReg:public Reg8<>{
        uint8_t ready:1;
        uint8_t ovl:1;
        uint8_t lock:1;
        uint8_t __resv__:5;
    };

    struct TemperatureReg:public Reg16<>{
        uint16_t data;
    };

    struct ConfigAReg:public Reg8<>{
        
        uint8_t measureMode:2;
        uint8_t dataRate:2;
        uint8_t fullScale:2;
        uint8_t OverSampleRatio:2;
        
    };

    struct ConfigBReg:public Reg8<>{
        
        uint8_t intEn:1;
        uint8_t __resv__:5;
        uint8_t rol:1;
        uint8_t srst:1;
        
    };

    struct ResetPeriodReg:public Reg8<>{
        using Reg8::operator=;
        uint8_t data;
    };

    struct ChipIDReg:public Reg8<>{
        uint8_t data;
    };

    enum class RegAddress:uint8_t{
        MagX = 0x00,
        MagY = 0x02,
        MagZ = 0x04,
        Status = 0x06,
        Tempature = 0x07,
        ConfigA = 0x09,
        ConfigB = 0x0A,
        ResetPeriod = 0x0B,
        ChipID = 0x0D
    };

    MagXReg magXReg;
    MagYReg magYReg;
    MagZReg magZReg;
    StatusReg statusReg;
    TemperatureReg temperatureReg;
    ConfigAReg configAReg;
    ConfigBReg configBReg;
    ResetPeriodReg resetPeriodReg;
    ChipIDReg chipIDReg;

    hal::HalResult write_reg(const RegAddress addr, const uint16_t data){
        return i2c_drv_.write_reg(uint8_t(addr), data, LSB);
    }

    hal::HalResult read_reg(const RegAddress addr, uint16_t & data){
        return i2c_drv_.read_reg(uint8_t(addr), data, LSB);
    }

    hal::HalResult write_reg(const RegAddress addr, const uint8_t data){
        return i2c_drv_.write_reg(uint8_t(addr), data);
    }

    hal::HalResult read_reg(const RegAddress addr, uint8_t & data){
        return i2c_drv_.read_reg(uint8_t(addr), data);
    }

    hal::HalResult read_burst(const RegAddress addr, int16_t * datas, const size_t len){
        return i2c_drv_.read_burst(uint8_t(addr), std::span(datas, len), LSB);
    }

    real_t From16BitToGauss(const int16_t data);

    void setFs(const FullScale FS){
        switch(FS){
        case FullScale::FS2G:
            fs = real_t(2);
            break;
        case FullScale::FS8G:
            fs = real_t(8);
            break;
        default:
            break;
        }
    }

    void setOvsfix(const OverSampleRatio OSR){
        switch(OSR){
        case OverSampleRatio::OSR512:
            ovsfix = 9;
            break;
        case OverSampleRatio::OSR256:
            ovsfix = 8;
            break;
        case OverSampleRatio::OSR128:
            ovsfix = 7;
            break;
        case OverSampleRatio::OSR64:
            ovsfix = 6;
            break;
        default:
            break;
        }
    }

    bool busy(){
        read_reg(RegAddress::Status, statusReg);
        return statusReg.ready == false;
    }
public:
    QMC5883L(const QMC5883L & other) = delete;
    QMC5883L(QMC5883L && other) = delete;

    QMC5883L(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    QMC5883L(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    QMC5883L(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
            i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    void init();

    void enableContMode(const bool en = true);
    
    void setDataRate(const DataRate rate);
    
    void setFullScale(const FullScale fullscale);
    
    void setOverSampleRatio(const OverSampleRatio ratio);

    void update();

    Option<Vector3_t<q24>> read_mag() override;
    
    bool validate();

    void setResetPeriod(const uint8_t resetPeriod);

    void reset();

    void enableInterrupt(const bool en = true);

    bool isOverflow();
};


}