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

    enum class Odr:uint8_t{
        DR0_75, DR1_5, DR3, DR7_5, DR15, DR30, DR75
    };

    enum class SampleNumber:uint8_t{
        SN1, SN2, SN4, SN8
    };

    enum class Gain:uint8_t{
        GL0_73, GL0_92, GL1_22, GL1_52, GL2_27, GL2_56, GL3_03, GL4_35
    };

    enum class Mode:uint8_t{
        Continuous = 0, 
        Single = 1
    };

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x3d);
};

struct HMC5883L_Regs:public HMC5883L_Collections{
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


    struct R8_ConfigA:public Reg8<>{
        static constexpr RegAddress address = RegAddress::ConfigA;
        uint8_t measureMode:3;
        uint8_t dataRate:2;
        uint8_t sampleNumber:2;
        uint8_t __resv__:1;
    }DEF_R8(config_a_reg)

    struct R8_ConfigB:public Reg8<>{
        static constexpr RegAddress address = RegAddress::ConfigB;
        uint8_t __resv__:5;
        Gain gain:3;
    }DEF_R8(config_b_reg)

    struct R8_Mode:public Reg8<>{
        static constexpr RegAddress address = RegAddress::Mode;
        Mode mode:2;
        uint8_t __resv__:5;
        uint8_t hs:1;
    }DEF_R8(mode_reg)

    struct R8_Status:public Reg8<>{
        static constexpr RegAddress address = RegAddress::Status;
        uint8_t ready:1;
        uint8_t lock:1;
        uint8_t __resv__:6;
    }DEF_R8(status_reg)

    struct R8_IdA:public Reg8<>{
        static constexpr RegAddress address = RegAddress::IDA;
        uint8_t data;
    }DEF_R8(id_a_reg)

    struct R8_IdB:public Reg8<>{
        static constexpr RegAddress address = RegAddress::IDB;
        uint8_t data;
    }DEF_R8(id_b_reg)

    struct R8_IdC:public Reg8<>{
        static constexpr RegAddress address = RegAddress::IDC;
        uint8_t data;
    }DEF_R8(id_c_reg)

    int16_t magXReg = {};
    int16_t magYReg = {};
    int16_t magZReg = {};
};

class HMC5883L final:
    public MagnetometerIntf,
    public HMC5883L_Regs{
public:
    HMC5883L(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    HMC5883L(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    HMC5883L(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> enable_high_speed(const bool en = true);

    [[nodiscard]] IResult<> set_odr(const Odr rate);
    [[nodiscard]] IResult<> set_sample_number(const SampleNumber number);

    [[nodiscard]] IResult<> set_gain(const Gain gain);
    [[nodiscard]] IResult<> set_mode(const Mode mode);

    [[nodiscard]] IResult<Vector3_t<q24>> read_mag();

    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> update();


    [[nodiscard]] IResult<bool> is_data_ready();
private:


    hal::I2cDrv i2c_drv_;

    real_t lsb_;


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(reg.address), reg.as_val(), MSB);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(reg.address), reg.as_ref(), MSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const RegAddress addr, int16_t * pdata, size_t len){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), std::span(pdata, len), MSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    void set_lsb(const Gain gain){
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

    static constexpr real_t transform_raw_to_gauss(const uint16_t data, const q24 lsb){
        return s16_to_uni(data & 0x8fff) * lsb;
    }
};

};