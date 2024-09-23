#pragma once

#include <optional>

#include "../drivers/device_defs.h"
#include "../drivers/IMU/IMU.hpp"
#include "types/uint24_t.h"

#pragma pack(push, 1)
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

#define REG16(x) (*reinterpret_cast<uint16_t *>(&x))
#define REG8(x) (*reinterpret_cast<uint8_t *>(&x))

// #define BMI150_DEBUG

#ifdef BMI150_DEBUG
#undef BMI150_DEBUG
#define BMI150_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define BMI150_DEBUG(...)
#endif

class BMI150:public Magnetometer{
public:


protected:
    std::optional<I2cDrv> i2c_drv;
    std::optional<SpiDrv> spi_drv;

    using RegAddress = uint8_t;
    static constexpr uint8_t default_chip_id = 0;//TODO
    static constexpr uint8_t default_i2c_addr = 0x68;

    struct{

    };


    void writeReg(const uint8_t addr, const uint8_t data){
        if(i2c_drv) i2c_drv->writeReg(addr, data);
        if(spi_drv){
            SpiDrv & drv = spi_drv.value();
            drv.write(uint8_t(addr), false);
            drv.write(data);

            BMI150_DEBUG("Wspi", addr, data);

        }
    }

    void readReg(const RegAddress addr, uint8_t & data){
        if(i2c_drv) i2c_drv->readReg((uint8_t)addr, data);
        if(spi_drv){
            SpiDrv & drv = spi_drv.value();
            drv.write(uint8_t(uint8_t(addr) | 0x80), false);
            drv.read(data);
        }

        BMI150_DEBUG("Rspi", addr, data);
    }

    void requestData(const RegAddress addr, void * datas, const size_t len){
        if(i2c_drv) i2c_drv->readPool(uint8_t(addr), (uint8_t *)datas, len);
        if(spi_drv){
            SpiDrv & drv = spi_drv.value();
            drv.write(uint8_t(uint8_t(addr) | 0x80), false);
            
            drv.read((uint8_t *)(datas), len);
        }

        BMI150_DEBUG("Rspi", addr, len);
    }

    void writeCommand(const uint8_t cmd){
        writeReg(0x7e, cmd);
    }
public:

    BMI150(const I2cDrv & _bus_drv):i2c_drv(_bus_drv){;}
    BMI150(I2cDrv && _bus_drv):i2c_drv(_bus_drv){;}
    BMI150(I2c & bus):i2c_drv(I2cDrv(bus, default_i2c_addr)){;}
    BMI150(const SpiDrv & _bus_drv):spi_drv(_bus_drv){;}
    BMI150(SpiDrv && _bus_drv):spi_drv(_bus_drv){;}
    BMI150(Spi & bus, const uint8_t index):spi_drv(SpiDrv(bus, index)){;}

    void init();
    void update();

    bool verify();

    void reset();

    std::tuple<real_t, real_t, real_t> getMagnet() override;
};


#pragma pack(pop)