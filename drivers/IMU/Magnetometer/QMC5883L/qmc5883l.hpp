#pragma once

#include "core/io/regs.hpp"
#include "core/utils/enum_array.hpp"
#include "drivers/IMU/IMU.hpp"
#include <tuple>

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct QMC5883L_Prelude{

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x1a);

    using Error = ImuError;
    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Mode:uint8_t{
        Single,Continuous
    };

    enum class Odr:uint8_t{
        _10, _50, _100, _200
    };

    enum class OverSampleRatio:uint8_t{
        _512, 
        _256, 
        _128, 
        _64
    };

    enum class FullScale:uint8_t{
        _2G, 
        _8G
    };
};

struct QMC5883L_Regs:public QMC5883L_Prelude{
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

    struct MagXReg:public Reg16i<>{
        static constexpr auto ADDRESS = RegAddress::MagX;
        int16_t :16;
    };

    struct MagYReg:public Reg16i<>{
        static constexpr auto ADDRESS = RegAddress::MagY;

        int16_t :16;
    };

    struct MagZReg:public Reg16i<>{
        static constexpr auto ADDRESS = RegAddress::MagZ;

        int16_t :16;
    };

    struct StatusReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Status;

        uint8_t ready:1;
        uint8_t ovl:1;
        uint8_t lock:1;
        uint8_t __resv__:5;
    };

    struct TemperatureReg:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::Tempature;

        uint16_t data;
    };

    struct ConfigAReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::ConfigA;
        uint8_t measureMode:2;
        Odr odr:2;
        FullScale fs:2;
        OverSampleRatio ovs_ratio:2;
        
    };

    struct ConfigBReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::ConfigB;

        uint8_t intEn:1;
        uint8_t __resv__:5;
        uint8_t rol:1;
        uint8_t srst:1;
        
    };

    struct ResetPeriodReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::ResetPeriod;
        using Reg8::operator=;
        uint8_t data;
    };

    struct ChipIDReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::ChipID;

        uint8_t data;
    };


    MagXReg mag_x_reg;
    MagYReg mag_y_reg;
    MagZReg mag_z_reg;
    StatusReg status_reg;
    TemperatureReg temperature_reg;
    ConfigAReg config_a_reg;
    ConfigBReg config_b_reg;
    ResetPeriodReg reset_period_reg;
    ChipIDReg chip_id_reg;

};


class QMC5883L:
    public MagnetometerIntf,
    public QMC5883L_Regs{
public:
    QMC5883L(const QMC5883L & other) = delete;
    QMC5883L(QMC5883L && other) = delete;

    QMC5883L(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    QMC5883L(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    QMC5883L(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> enable_cont_mode(const Enable en = EN);
    
    [[nodiscard]] IResult<> set_odr(const Odr rate);
    
    [[nodiscard]] IResult<> set_fs(const FullScale fullscale);
    
    [[nodiscard]] IResult<> set_over_sample_ratio(const OverSampleRatio ratio);

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<Vector3<q24>> read_mag() override;
    
    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> set_reset_period(const uint8_t resetPeriod);

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> enable_interrupt(const Enable en = EN);

    [[nodiscard]] IResult<bool> is_overflow();
private:
    hal::I2cDrv i2c_drv_;

    static constexpr EnumArray<FullScale, q24> scaler_mapping_ = {
        2, 8
    };

    static constexpr EnumScaler<FullScale, q24> scaler_ = {
        FullScale::_2G,
        scaler_mapping_
    };


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(T::ADDRESS), reg.as_val(), LSB);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(T::ADDRESS), reg.as_ref(), LSB);
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }


    [[nodiscard]] IResult<> read_burst(
        const RegAddress addr, 
        std::span<int16_t> pbuf
    ){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf, LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


    IResult<bool> is_busy(){
        if(const auto res = read_reg(status_reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(status_reg.ready == false);
    }
};


}