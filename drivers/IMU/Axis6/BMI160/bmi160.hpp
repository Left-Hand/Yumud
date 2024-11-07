#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"

// #define BMI160_DEBUG

#ifdef BMI160_DEBUG
#undef BMI160_DEBUG
#define BMI160_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define BMI160_DEBUG(...)
#endif

namespace yumud::drivers{

class BMI160:public Axis6{
public:

    enum class DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class G:uint8_t{
        _2, _4, _8, _16
    };

    enum class Command:uint8_t{
        START_FOC = 0x04,
        ACC_SET_PMU = 0b0001'0000,
        GYRO_SET_PMU = 0b0001'0100,
        MAG_SET_PMU = 0b0001'1000,
        FIFO_FLUSH = 0xB0,
        RESET_INTERRUPT =0xB1,
        SOFT_RESET = 0xB1,
        STEP_CNT_CLR = 0xB2
    };

    enum class PmuType{
        ACC,
        GYRO,
        MAG
    };

    enum class PmuMode{
        SUSPEND,
        NORMAL,
        LOW_POWER,
        FAST_SETUP
    };

protected:
    std::optional<I2cDrv> i2c_drv;
    std::optional<SpiDrv> spi_drv;

    scexpr uint8_t default_chip_id = 0;//TODO
    scexpr uint8_t default_i2c_addr = 0x68;

    #include "regs.ipp"

    struct{
        ChipIdReg chip_id_reg;
        uint8_t __resv1__;
        ErrReg err_reg;
        PmuStatusReg pmu_status_reg;
        StatusReg status_reg;
        Vector3i16Reg mag_reg;
        RhallReg rhall_reg;
        Vector3i16Reg gyro_reg;
        Vector3i16Reg accel_reg;
    };


    void writeReg(const uint8_t addr, const uint8_t data){
        if(i2c_drv) i2c_drv->writeReg(addr, data, MSB);
        if(spi_drv){
            spi_drv->writeSingle(uint8_t(addr), CONT);
            spi_drv->writeSingle(data);

            BMI160_DEBUG("Wspi", addr, data);

        }
    }

    void readReg(const RegAddress addr, uint8_t & data){
        if(i2c_drv) i2c_drv->readReg((uint8_t)addr, data, MSB);
        if(spi_drv){
            spi_drv->writeSingle(uint8_t(uint8_t(addr) | 0x80), CONT);
            spi_drv->readSingle(data);
        }

        BMI160_DEBUG("Rspi", addr, data);
    }

    void requestData(const RegAddress addr, void * datas, const size_t len){
        if(i2c_drv) i2c_drv->readMulti(uint8_t(addr), (uint8_t *)datas, len, MSB);
        if(spi_drv){
            spi_drv->writeSingle(uint8_t(uint8_t(addr) | 0x80), CONT);
            spi_drv->readMulti((uint8_t *)(datas), len);
        }

        BMI160_DEBUG("Rspi", addr, len);
    }

    void writeCommand(const uint8_t cmd){
        writeReg(0x7e, cmd);
    }
public:

    BMI160(const I2cDrv & _bus_drv):i2c_drv(_bus_drv){;}
    BMI160(I2cDrv && _bus_drv):i2c_drv(_bus_drv){;}
    BMI160(I2c & bus):i2c_drv(I2cDrv(bus, default_i2c_addr)){;}
    BMI160(const SpiDrv & _bus_drv):spi_drv(_bus_drv){;}
    BMI160(SpiDrv && _bus_drv):spi_drv(_bus_drv){;}
    BMI160(Spi & bus, const uint8_t index):spi_drv(SpiDrv(bus, index)){;}

    void init();
    void update();

    bool verify();

    void reset();

    void setPmuMode(const PmuType pum, const PmuMode mode);
    PmuMode getPmuMode(const PmuType pum);
    std::tuple<real_t, real_t, real_t> getAccel() override;
    std::tuple<real_t, real_t, real_t> getGyro() override;
};

}
