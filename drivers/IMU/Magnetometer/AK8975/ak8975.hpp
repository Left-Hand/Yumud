#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/AsahiKaseiIMU.hpp"


namespace ymd::drivers{


class AK8975_Prelude{
public:
    using Error = ImuError;

    enum class Mode:uint8_t{
        PowerDown = 0b0000,
        SingleMeasurement = 0b0001,
        SelfTest = 0b1000,
        FuseRomAccess = 0b1111,
    };


    using RegAddr = uint8_t;

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr uint8_t CORRECT_CHIP_ID = 0x48;
    static constexpr  auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x68 >> 1);
};


class AK8975 final:
    public AK8975_Prelude{
public:
public:

    explicit AK8975(const hal::I2cDrv & i2c_drv):
        transport_(i2c_drv){;}
    explicit AK8975(hal::I2cDrv && i2c_drv):
        transport_(i2c_drv){;}
    explicit AK8975(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        transport_(hal::I2cDrv(i2c, addr)){;}
    explicit AK8975(const hal::SpiDrv & spi_drv):
        transport_(spi_drv){;}
    explicit AK8975(hal::SpiDrv && spi_drv):
        transport_(std::move(spi_drv)){;}
    explicit AK8975(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        transport_(hal::SpiDrv(spi, rank)){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<bool> is_busy();
    [[nodiscard]] IResult<bool> is_stable();
    [[nodiscard]] IResult<> set_mode(const Mode mode);
    [[nodiscard]] IResult<> disable_i2c();
    [[nodiscard]] IResult<math::Vec3<iq24>> read_mag() ;
private:
    AsahiKaseiImu_Transport transport_;
    struct{
        int16_t x;
        int16_t y;
        int16_t z;

        uint8_t x_adj;
        uint8_t y_adj;
        uint8_t z_adj;
    };

    [[nodiscard]] IResult<> update_adj();
};

};