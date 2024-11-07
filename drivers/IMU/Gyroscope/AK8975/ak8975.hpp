#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"

// #define AK8975_DEBUG

#ifdef AK8975_DEBUG
#undef AK8975_DEBUG
#define AK8975_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define AK8975_DEBUG(...)
#endif


namespace yumud::drivers{
class AK8975:public Magnetometer{
public:
    enum class Mode:uint8_t{
        PowerDown = 0b0000,
        SingleMeasurement = 0b0001,
        SelfTest = 0b1000,
        FuseRomAccess = 0b1111,
    };

protected:
    std::optional<I2cDrv> i2c_drv;
    std::optional<SpiDrv> spi_drv;

    using RegAddress = uint8_t;
    scexpr uint8_t default_chip_id = 0;//TODO
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

    AK8975(const I2cDrv & _bus_drv):i2c_drv(_bus_drv){;}
    AK8975(I2cDrv && _bus_drv):i2c_drv(_bus_drv){;}
    AK8975(I2c & bus):i2c_drv(I2cDrv(bus, default_i2c_addr)){;}
    AK8975(const SpiDrv & _bus_drv):spi_drv(_bus_drv){;}
    AK8975(SpiDrv && _bus_drv):spi_drv(_bus_drv){;}
    AK8975(Spi & bus, const uint8_t index):spi_drv(SpiDrv(bus, index)){;}

    void init();
    void update();
    bool verify();
    bool isIdle();
    bool stable();
    void setMode(const Mode mode);
    void disableI2c();
    std::tuple<real_t, real_t, real_t> getMagnet() override;
};

};