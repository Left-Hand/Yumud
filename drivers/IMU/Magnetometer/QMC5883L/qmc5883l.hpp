#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include <tuple>

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{

struct QMC5883L_Collections{
    enum class Mode:uint8_t{
        Single,Continuous
    };

    enum class DataRate:uint8_t{
        DR10, DR50, DR100, DR200
    };

    enum class OverSampleRatio:uint8_t{
        OSR512, OSR256, OSR128, OSR64
    };

    enum class FullScale:uint8_t{
        FS2G, FS8G
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

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x1a);

    using Error = ImuError;
    template<typename T = void>
    using IResult = Result<T, Error>;
};

struct QMC5883L_Regs:public QMC5883L_Collections{

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


    MagXReg magXReg;
    MagYReg magYReg;
    MagZReg magZReg;
    StatusReg statusReg;
    TemperatureReg temperatureReg;
    ConfigAReg configAReg;
    ConfigBReg configBReg;
    ResetPeriodReg resetPeriodReg;
    ChipIDReg chipIDReg;

};


class QMC5883L:
    public MagnetometerIntf,
    public QMC5883L_Regs{
public:
    QMC5883L(const QMC5883L & other) = delete;
    QMC5883L(QMC5883L && other) = delete;

    QMC5883L(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    QMC5883L(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    QMC5883L(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
            i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> enable_cont_mode(const bool en = true);
    
    [[nodiscard]] IResult<> set_data_rate(const DataRate rate);
    
    [[nodiscard]] IResult<> set_full_scale(const FullScale fullscale);
    
    [[nodiscard]] IResult<> set_over_sample_ratio(const OverSampleRatio ratio);

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<Vector3_t<q24>> read_mag() override;
    
    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> set_reset_period(const uint8_t resetPeriod);

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> enable_interrupt(const bool en = true);

    [[nodiscard]] IResult<bool> is_overflow();
private:
    hal::I2cDrv i2c_drv_;

    real_t fs;
    uint8_t ovsfix = 0;

    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const uint16_t data){
        if(const auto res = i2c_drv_.write_reg(uint8_t(addr), data, LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const RegAddress addr, uint16_t & data){
        if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data, LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const uint8_t data){
        if(const auto res = i2c_drv_.write_reg(uint8_t(addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const RegAddress addr, uint8_t & data){
        if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const RegAddress addr, int16_t * datas, const size_t len){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), std::span(datas, len), LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


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

    IResult<bool> is_busy(){
        if(const auto res = read_reg(RegAddress::Status, statusReg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(statusReg.ready == false);
    }
};


}