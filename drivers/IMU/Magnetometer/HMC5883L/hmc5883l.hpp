#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct HMC5883L_Collections{
    using Error = ImuError;
    template<typename T = void>
    using IResult = Result<T, Error>;


    enum class DataRate:uint8_t{
        DR0_75, DR1_5, DR3, DR7_5, DR15, DR30, DR75
    };

    enum class SampleNumber:uint8_t{
        SN1, SN2, SN4, SN8
    };

    enum class MeasurementMode:uint8_t{
        Norm, Pos, Neg
    };

    enum class Gain:uint8_t{
        GL0_73, GL0_92, GL1_22, GL1_52, GL2_27, GL2_56, GL3_03, GL4_35
    };

    enum class Mode:uint8_t{
        Continuous, Single
    };

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x3d);
};

struct HMC5883L_Regs:public HMC5883L_Collections{
    struct ConfigAReg:public Reg8<>{
        
        uint8_t measureMode:3;
        uint8_t dataRate:2;
        uint8_t sampleNumber:2;
        uint8_t __resv__:1;
        
    };

    struct ConfigBReg:public Reg8<>{
        
        uint8_t __resv__:5;
        uint8_t gain:3;
        
    };

    struct ModeReg:public Reg8<>{
        
        uint8_t mode:2;
        uint8_t __resv__:5;
        uint8_t hs:1;
        
    };

    struct StatusReg:public Reg8<>{
        
        uint8_t ready:1;
        uint8_t lock:1;
        uint8_t __resv__:6;
        
    };

    enum class RegAddress:uint8_t{
        ConfigA = 0x00,
        ConfigB = 0x01,
        Mode = 0x02,
        MagX = 0x03,
        MagY = 0x05,
        MagZ = 0x07,
        Status = 0x09,
        IDA = 10,
        IDB = 11,
        IDC = 12
    };

    ConfigAReg configAReg = {};
    ConfigBReg configBReg = {};
    ModeReg modeReg = {};
    int16_t magXReg = {};
    int16_t magYReg = {};
    int16_t magZReg = {};
    StatusReg statusReg = {};
};

class HMC5883L final:
    public MagnetometerIntf,
    public HMC5883L_Regs{
public:
    HMC5883L(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    HMC5883L(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    HMC5883L(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    void init();
    void enableHighSpeed(const bool en = true);

    void setMeasurementMode(const MeasurementMode mode);

    void setDataRate(const DataRate rate);
    void setSampleNumber(const SampleNumber number);

    void setGain(const Gain gain);
    void setMode(const Mode mode);

    IResult<Vector3_t<q24>> read_mag() override;

    bool validate();
    void update();


    bool busy();

    void enableContMode(const bool en = true);
private:
    static constexpr real_t From12BitToGauss(const uint16_t data, const q24 lsb){
        return s16_to_uni(data & 0x8fff) * lsb;
    }

    hal::I2cDrv i2c_drv_;

    real_t lsb_;



    hal::HalResult write_reg(const RegAddress addr, const uint16_t data){
        return i2c_drv_.write_reg(uint8_t(addr), data, MSB);
    }

    hal::HalResult read_reg(const RegAddress addr, uint16_t & data){
        return i2c_drv_.read_reg(uint8_t(addr), data, MSB);
    }

    hal::HalResult write_reg(const RegAddress addr, const uint8_t data){
        return i2c_drv_.write_reg(uint8_t(addr), data);
    }

    hal::HalResult read_reg(const RegAddress addr, uint8_t & data){
        return i2c_drv_.read_reg(uint8_t(addr), data);
    }


    hal::HalResult read_burst(const RegAddress addr, int16_t * pdata, size_t len){
        return i2c_drv_.read_burst(uint8_t(addr), std::span(pdata, len), MSB);
    }



    void setLsb(const Gain gain){
        lsb_ = transfrom_gain_into_lsb(gain);
    }

    static constexpr q24 transfrom_gain_into_lsb(const Gain gain){
        switch(gain){
        case Gain::GL0_73:
            return q24(0.73);
        case Gain::GL0_92:
            return q24(0.92);
        case Gain::GL1_22:
            return q24(1.22);
        case Gain::GL1_52:
            return q24(1.52);
        case Gain::GL2_27:
            return q24(2.27);
        case Gain::GL2_56:
            return q24(2.56);
        case Gain::GL3_03:
            return q24(3.03);
        case Gain::GL4_35:
            return q24(4.35);
        default: __builtin_unreachable();
        }
    }
};

};