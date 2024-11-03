#pragma once

#include <optional>

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"



// #define BMM150_DEBUG

#ifdef BMM150_DEBUG
#undef BMM150_DEBUG
#define BMM150_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define BMM150_DEBUG(...)
#endif

namespace yumud::drivers{

class BMM150:public Magnetometer{
public:


protected:
    std::optional<I2cDrv> i2c_drv;
    std::optional<SpiDrv> spi_drv;

    using RegAddress = uint8_t;
    scexpr uint8_t default_chip_id = 0;//TODO
    scexpr uint8_t default_i2c_addr = 0x68;

    struct{

    };


    void writeReg(const uint8_t addr, const uint8_t data){
        if(i2c_drv) i2c_drv->writeReg(addr, data, MSB);
        if(spi_drv){
            spi_drv->write(uint8_t(addr), false);
            spi_drv->write(data);

            BMM150_DEBUG("Wspi", addr, data);

        }
    }

    void readReg(const RegAddress addr, uint8_t & data){
        if(i2c_drv) i2c_drv->readReg((uint8_t)addr, data, MSB);
        if(spi_drv){
            spi_drv->write(uint8_t(uint8_t(addr) | 0x80), false);
            spi_drv->read(data);
        }

        BMM150_DEBUG("Rspi", addr, data);
    }

    void requestData(const RegAddress addr, void * datas, const size_t len){
        if(i2c_drv) i2c_drv->readPool(uint8_t(addr), (uint8_t *)datas, len, MSB);
        if(spi_drv){
            spi_drv->write(uint8_t(uint8_t(addr) | 0x80), false);
            spi_drv->read((uint8_t *)(datas), len);
        }

        BMM150_DEBUG("Rspi", addr, len);
    }

    void writeCommand(const uint8_t cmd){
        writeReg(0x7e, cmd);
    }
public:

    BMM150(const I2cDrv & _bus_drv):i2c_drv(_bus_drv){;}
    BMM150(I2cDrv && _bus_drv):i2c_drv(_bus_drv){;}
    BMM150(I2c & bus):i2c_drv(I2cDrv(bus, default_i2c_addr)){;}
    BMM150(const SpiDrv & _bus_drv):spi_drv(_bus_drv){;}
    BMM150(SpiDrv && _bus_drv):spi_drv(_bus_drv){;}
    BMM150(Spi & bus, const uint8_t index):spi_drv(SpiDrv(bus, index)){;}

    void init();
    void update();

    bool verify();

    void reset();

    std::tuple<real_t, real_t, real_t> getMagnet() override;
};


}