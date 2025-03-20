#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"

namespace ymd::drivers{

class BMI270:public Axis6{
public:
    using Error = details::BoschSensorError;

    enum class DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class G:uint8_t{
        _2, _4, _8, _16
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
    BoschSensor_Phy phy_;

    scexpr uint8_t default_i2c_addr = 0x68;

    using RegAddress = uint8_t;
    
    #include "regs.ipp"

    ChipIdReg chipid_reg = {};
    uint8_t :8;
    ErrReg err_reg = {};
    StatusReg status_reg = {};

    uint8_t data_regs[Data0Reg::size] = {};
    uint8_t sensor_time_regs[SensorTime0Reg::size] = {};

    EventReg event_reg = {};
    
    IntStatus0Reg int_status0_reg = {};
    IntStatus1Reg int_status1_reg = {};

public:
    BMI270(hal::I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):
        phy_(hal::I2cDrv{i2c, default_i2c_addr}){;}


    void init();
    void update();
    bool verify();
    void reset();

    void setPmuMode(const PmuType pum, const PmuMode mode);
    PmuMode getPmuMode(const PmuType pum);
    Option<Vector3_t<real_t>> getAcc();
    Option<Vector3_t<real_t>> getGyr();
};

}
