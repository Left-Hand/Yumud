#pragma once

#include "core/io/regs.hpp"

#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/STMicroIMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{

class LIS2DW12:public Accelerometer, public STMicroSensor{
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

    enum class GyrOdr:uint8_t{
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
    scexpr uint8_t default_i2c_addr = 0b11010010;

    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;

    real_t acc_scale = 0;
    real_t gyr_scale = 0;

    using RegAddress = uint8_t;
    struct TempOutReg:public Reg16<>{scexpr RegAddress address = 0x0D; uint8_t data;};
    struct WhoAmIReg:public Reg8<>{scexpr RegAddress address = 0x0F; uint8_t data;};
    struct Ctrl1Reg:public Reg8<>{
        scexpr RegAddress address = 0x20;
        uint8_t lp_mode:2;
        uint8_t mode:2;
        uint8_t odr:4;    
    };

    struct Ctrl2Reg:public Reg8<>{
        scexpr RegAddress address = 0x21;
        uint8_t sim:1;
        uint8_t i2c_disable:1;
        uint8_t if_add_inc:1;
        uint8_t bdu:1;
        uint8_t cs_pu_disc:1;
        uint8_t :1;
        uint8_t soft_reset:1;
        uint8_t boot:1;
    };

    struct Ctrl3Reg:public Reg8<>{
        scexpr RegAddress address = 0x22;
        uint8_t slp_mode_1:1;
        uint8_t slp_mode_sel:1;
        uint8_t :1;
        uint8_t h_active:1;
        uint8_t lir:1;
        uint8_t pp_od:1;
        uint8_t selftest_mode:2;
    };

    struct Ctrl4Reg:public Reg8<>{
        scexpr RegAddress address = 0x23;
        uint8_t int1_drdy:1;
        uint8_t int1_fth:1;
        uint8_t int1_diff5:1;
        uint8_t int1_tap:1;
        uint8_t int1_ff:1;
        uint8_t int1_wu:1;
        uint8_t int1_single_tap:1;
        uint8_t int1_6d:1;
    };

    struct Ctrl5Reg:public Reg8<>{
        scexpr RegAddress address = 0x24;
        uint8_t int2_drdy:1;
        uint8_t int2_fth:1;
        uint8_t int2_diff5:1;
        uint8_t int2_ovr:1;
        uint8_t int2_drdy_t:1;
        uint8_t int2_boot:1;
        uint8_t int2_sleep_chg:1;
        uint8_t int2_sleep_state:1;
    };

    struct Ctrl6Reg:public Reg8<>{
        scexpr RegAddress address = 0x25;
        uint8_t :2;
        uint8_t low_noise:1;
        uint8_t fds:1;
        uint8_t fs:2;
        uint8_t bw_filt:2;
    };

    struct TempOut8Reg:public Reg16<>{scexpr RegAddress address = 0x26; uint8_t data;};

    struct StatusReg:public Reg8<>{
        scexpr RegAddress address = 0x27;
        uint8_t drdy:1;
        uint8_t ff_ia:1;
        uint8_t _6d_ia:1;
        uint8_t single_tap:1;
        uint8_t double_tap:1;
        uint8_t sleep_state:1;
        uint8_t wu_ia:1;
        uint8_t fifo_ths:1;
    };

    struct OutputXReg:public Reg16i<>{scexpr RegAddress address = 0x28; int16_t data;};
    struct OutputYReg:public Reg16i<>{scexpr RegAddress address = 0x2A; int16_t data;};
    struct OutputZReg:public Reg16i<>{scexpr RegAddress address = 0x2C; int16_t data;};

    struct FifoCtrlReg:public Reg8<>{
        scexpr RegAddress address = 0x2E;
        uint8_t fth:5;
        uint8_t fmode:3;
    };

    struct FifoSamplesReg:public Reg8<>{
        scexpr RegAddress address = 0x2F;
        uint8_t diff0:6;
        uint8_t fifo_ovr:1;
        uint8_t fifo_fth:1;
    };



    static real_t calculateAccScale(const AccRange range);
    static real_t calculateGyrScale(const GyrRange range);
public:

    LIS2DW12(hal::I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):STMicroSensor(hal::I2cDrv{i2c, i2c_addr}){;}
    LIS2DW12(const hal::I2cDrv & i2c_drv):STMicroSensor(i2c_drv){;}
    LIS2DW12(hal::I2cDrv && i2c_drv):STMicroSensor(std::move(i2c_drv)){;}
    LIS2DW12(const hal::SpiDrv & spi_drv):STMicroSensor(spi_drv){;}
    LIS2DW12(hal::SpiDrv && spi_drv):STMicroSensor(std::move(spi_drv)){;}
    LIS2DW12(hal::Spi & spi, const uint8_t index):STMicroSensor(hal::SpiDrv{spi, index}){;}

    void init();
    void update();

    bool verify();

    void reset();

    void setAccOdr(const AccOdr odr);
    void setAccRange(const AccRange range);
    real_t setGyrOdr(const real_t odr);
    
    void setPmuMode(const PmuType pum, const PmuMode mode);
    PmuMode getPmuMode(const PmuType pum);
    Option<Vector3_t<real_t>> getAcc();
};

}
