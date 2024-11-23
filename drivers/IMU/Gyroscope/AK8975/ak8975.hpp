#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"


namespace ymd::drivers{
class AK8975:public Magnetometer{
public:
    enum class Mode:uint8_t{
        PowerDown = 0b0000,
        SingleMeasurement = 0b0001,
        SelfTest = 0b1000,
        FuseRomAccess = 0b1111,
    };

protected:
    std::optional<I2cDrv> i2c_drv_;
    std::optional<SpiDrv> spi_drv_;

    using RegAddress = uint8_t;

    scexpr uint8_t default_i2c_addr = 0x68;

    struct{
        int16_t x;
        int16_t y;
        int16_t z;

        uint8_t x_adj;
        uint8_t y_adj;
        uint8_t z_adj;
    };


    void writeReg(const uint8_t addr, const uint8_t data);
    void readReg(const RegAddress addr, uint8_t & data);

    void readMulti(const RegAddress addr, void * datas, const size_t len);
    void readAdj();
public:

    AK8975(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    AK8975(I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    AK8975(I2c & bus):i2c_drv_(I2cDrv(bus, default_i2c_addr)){;}
    AK8975(const SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    AK8975(SpiDrv && spi_drv):spi_drv_(std::move(spi_drv)){;}
    AK8975(Spi & bus, const uint8_t index):spi_drv_(SpiDrv(bus, index)){;}

    void init();
    void update();
    bool verify();
    bool busy();
    bool stable();
    void setMode(const Mode mode);
    void disableI2c();
    std::tuple<real_t, real_t, real_t> getMagnet() override;
};

};