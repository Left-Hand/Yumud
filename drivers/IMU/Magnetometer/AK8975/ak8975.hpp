#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/AsahiKaseiIMU.hpp"


namespace ymd::drivers{

class AK8975 final:public MagnetometerIntf{
public:
    using Error = ImuError;

    enum class Mode:uint8_t{
        PowerDown = 0b0000,
        SingleMeasurement = 0b0001,
        SelfTest = 0b1000,
        FuseRomAccess = 0b1111,
    };

protected:
    using RegAddress = uint8_t;



    template<typename T = void>
    using IResult = Result<T, Error>;



    AsahiKaseiSensor_Phy phy_;
    struct{
        int16_t x;
        int16_t y;
        int16_t z;

        uint8_t x_adj;
        uint8_t y_adj;
        uint8_t z_adj;
    };

    [[nodiscard]] IResult<> update_adj();
public:
    static constexpr  auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x68);
    AK8975(const hal::I2cDrv & i2c_drv):phy_(i2c_drv){;}
    AK8975(hal::I2cDrv && i2c_drv):phy_(i2c_drv){;}
    AK8975(hal::I2c & bus):phy_(hal::I2cDrv(bus, DEFAULT_I2C_ADDR)){;}
    AK8975(const hal::SpiDrv & spi_drv):phy_(spi_drv){;}
    AK8975(hal::SpiDrv && spi_drv):phy_(std::move(spi_drv)){;}
    AK8975(hal::Spi & spi, const hal::SpiSlaveIndex index):phy_(hal::SpiDrv(spi, index)){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<bool> is_busy();
    [[nodiscard]] IResult<bool> is_stable();
    [[nodiscard]] IResult<> set_mode(const Mode mode);
    [[nodiscard]] IResult<> disable_i2c();
    [[nodiscard]] IResult<Vector3<q24>> read_mag() ;
};

};