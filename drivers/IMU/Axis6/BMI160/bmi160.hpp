#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"

namespace ymd::drivers{

class BMI160:public Axis6{
public:

    enum class DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class G:uint8_t{
        _2, _4, _8, _16
    };

    enum class AccOdr:uint8_t{
        _25_32 = 0b0001,
        _25_16,
        _25_8,
        _25_4,
        _25_2,
        _25,
        
        _50,
        _100,
        _200,
        _400,
        _800,
        _1600
    };

    enum class Gyrdr:uint8_t{
        _25 = 0b0110,
        
        _50,
        _100,
        _200,
        _400,
        _800,
        _1600,
        _3200
    };
    
    enum class AccRange:uint8_t{
        _2G     =   0b0011,
        _4G     =   0b0101,
        _8G     =   0b1000,
        _16G    =   0b1100
    };

    enum class GyrRange:uint8_t{
        _2000deg = 0b0000,
        _1000deg,
        _500deg,
        _250deg,
        _125deg
    };

    enum class Command:uint8_t{
        START_FOC = 0x04,
        ACC_SET_PMU = 0b0001'0000,
        GYR_SET_PMU = 0b0001'0100,
        MAG_SET_PMU = 0b0001'1000,
        FIFO_FLUSH = 0xB0,
        RESET_INTERRUPT =0xB1,
        SOFT_RESET = 0xB1,
        STEP_CNT_CLR = 0xB2
    };

    enum class PmuType{
        ACC,
        GYR,
        MAG
    };

    enum class PmuMode{
        SUSPEND,
        NORMAL,
        LOW_POWER,
        FAST_SETUP
    };

protected:
    std::optional<I2cDrv> i2c_drv_;
    std::optional<SpiDrv> spi_drv_;

    scexpr uint8_t default_i2c_addr = 0b11010010;
    // scexpr uint8_t default_i2c_addr = 0b11010000;
    // scexpr uint8_t default_i2c_addr = 0x69;

    real_t acc_scale = 0;
    real_t gyr_scale = 0;

    #include "regs.ipp"

    struct{
        ChipIdReg chip_id_reg;
        uint8_t __resv1__;
        ErrReg err_reg;
        PmuStatusReg pmu_status_reg;
        StatusReg status_reg;
        Vector3i16Reg mag_reg;
        RhallReg rhall_reg;
        Vector3i16Reg gyr_reg;
        Vector3i16Reg acc_reg;

        AccConfReg acc_conf_reg;
        AccRangeReg acc_range_reg;
        GyrConfReg gyr_conf_reg;
        GyrRangeReg gyr_range_reg;
    };


    void writeReg(const uint8_t addr, const uint8_t data);

    void readReg(const RegAddress addr, uint8_t & data);

    void requestData(const RegAddress addr, int16_t * datas, const size_t len);

    void writeCommand(const uint8_t cmd){
        writeReg(0x7e, cmd);
    }

    static real_t calculateAccScale(const AccRange range);
    static real_t calculateGyrScale(const GyrRange range);
public:

    BMI160(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    BMI160(I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    BMI160(I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(I2cDrv(i2c, i2c_addr)){;}
    BMI160(const SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    BMI160(SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    BMI160(Spi & spi, const uint8_t index):spi_drv_(SpiDrv(spi, index)){;}

    void init();
    void update();

    bool verify();

    void reset();

    void setAccOdr(const AccOdr odr);
    void setAccRange(const AccRange range);
    void setGyrOdr(const Gyrdr odr);
    void setGyrRange(const GyrRange range);
    
    void setPmuMode(const PmuType pum, const PmuMode mode);
    PmuMode getPmuMode(const PmuType pum);
    std::tuple<real_t, real_t, real_t> getAcc() override;
    std::tuple<real_t, real_t, real_t> getGyr() override;
};

}
