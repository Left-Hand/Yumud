

#pragma once

#include <optional>

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"
#include "types/uint24_t.h"


// #define ICM42688_DEBUG

#ifdef ICM42688_DEBUG
#undef ICM42688_DEBUG
#define ICM42688_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define ICM42688_DEBUG(...)
#endif

namespace yumud::drivers{

class ICM42688:public Axis6{
public:


protected:
    std::optional<I2cDrv> i2c_drv;
    std::optional<SpiDrv> spi_drv;

    using RegAddress = uint8_t;


    scexpr uint8_t default_chip_id = 0;//TODO
    scexpr uint8_t default_i2c_addr = 0x68;

    struct Vec3i16{
        int16_t x;
        int16_t y;
        int16_t z;
    };
    
    struct{
        Vec3i16 accel_data;
        Vec3i16 gyro_data;
    };


    void writeReg(const uint8_t addr, const uint8_t data){
        if(i2c_drv) i2c_drv->writeReg(addr, data);
        if(spi_drv){
            SpiDrv & drv = spi_drv.value();
            drv.write(uint8_t(addr), false);
            drv.write(data);

            ICM42688_DEBUG("Wspi", addr, data);

        }
    }

    void readReg(const RegAddress addr, uint8_t & data){
        if(i2c_drv) i2c_drv->readReg((uint8_t)addr, data);
        if(spi_drv){
            SpiDrv & drv = spi_drv.value();
            drv.write(uint8_t(uint8_t(addr) | 0x80), false);
            drv.read(data);
        }

        ICM42688_DEBUG("Rspi", addr, data);
    }


    void writeReg(const uint8_t addr, const uint16_t data){
        if(i2c_drv) i2c_drv->writeReg(addr, data);
        if(spi_drv){
            SpiDrv & drv = spi_drv.value();
            drv.write(uint8_t(addr), false);
            drv.write(data);

            ICM42688_DEBUG("Wspi", addr, data);

        }
    }

    void readReg(const RegAddress addr, uint16_t & data){
        if(i2c_drv) i2c_drv->readReg((uint8_t)addr, data);
        if(spi_drv){
            SpiDrv & drv = spi_drv.value();
            drv.write(uint8_t(uint8_t(addr) | 0x80), false);
            drv.read(data);
        }

        ICM42688_DEBUG("Rspi", addr, data);
    }

    void requestData(const RegAddress addr, void * datas, const size_t len){
        if(i2c_drv) i2c_drv->readPool(uint8_t(addr), (uint8_t *)datas, len);
        if(spi_drv){
            SpiDrv & drv = spi_drv.value();
            drv.write(uint8_t(uint8_t(addr) | 0x80), false);
            
            drv.read((uint8_t *)(datas), len);
        }

        ICM42688_DEBUG("Rspi", addr, len);
    }

    void writeCommand(const uint8_t cmd){
        writeReg(0x7e, cmd);
    }
public:

    ICM42688(const I2cDrv & _bus_drv):i2c_drv(_bus_drv){;}
    ICM42688(I2cDrv && _bus_drv):i2c_drv(_bus_drv){;}
    ICM42688(I2c & bus):i2c_drv(I2cDrv(bus, default_i2c_addr)){;}
    ICM42688(const SpiDrv & _bus_drv):spi_drv(_bus_drv){;}
    ICM42688(SpiDrv && _bus_drv):spi_drv(_bus_drv){;}
    ICM42688(Spi & bus, const uint8_t index):spi_drv(SpiDrv(bus, index)){;}

    void init();
    
    void update();

    bool verify();

    void reset();

    std::tuple<real_t, real_t, real_t> getAccel() override;
    std::tuple<real_t, real_t, real_t> getGyro() override;
};

}
