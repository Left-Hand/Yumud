#pragma once

#include <optional>

#include "core/io/regs.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"


namespace ymd::drivers{

struct BMM350_Prelude{
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class DataRate:uint8_t{
        _400Hz = 0x02,
        _200Hz = 0x03,
        _100Hz = 0x04,
        _50Hz = 0x05,
        _25Hz = 0x06,
        _12_5Hz = 0x07,
        _6_25Hz = 0x08,
        _3_125Hz = 0x09,
        _1_5625Hz = 0x0a
    };

    enum class AverageTimes:uint8_t{
        _0 = 0x00,
        _2 = 0x01,
        _4 = 0x02,
        _8 = 0x03,
    };

    enum class PmuCommand:uint8_t{
        Suspend = 0x00,
        Normal = 0x01,
        UpdateOAE = 0x02,
        ForcedWithFullCRST = 0x03,
        ForcedWithFastCRST = 0x04,
        FluxGuideWithFullCRST = 0x05,
        FluxGuideWithFastCRST = 0x06,
        BitResetWithFullCRST = 0x07,
        BitResetWithFastCRST = 0x08,
    };

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x68 >> 1);
};


struct BMM350_Regset:public BMM350_Prelude{

//0x00
struct R8_ChipId:public Reg8<>{
    static constexpr uint8_t KEY = 0x33;
    uint8_t chip_id_top:4;
    uint8_t chip_id_fixed:4;
};

//0x02
struct R8_Err:public Reg8<>{
    uint8_t pmu_cmd_error:1;
    uint8_t :7;
};


//0x03
struct R8_PadCtrl:public Reg8<>{
    uint8_t drv:3;
    uint8_t :5;
};


//0x04
struct R8_PmuAddr:public Reg8<>{
    DataRate odr:4;
    AverageTimes avg:2;
    uint8_t :2;
};

//0x05
struct R8_PmuAxisEn:public Reg8<>{
    uint8_t x:1;
    uint8_t y:1;
    uint8_t z:1;
    uint8_t :5;
};

//0x06
struct R8_PmuCmd:public Reg8<>{
    PmuCommand pmu_cmd:4;
    uint8_t :4;
};

//0x07
struct R8_PmuCmdStatus:public Reg8<>{
    uint8_t pmu_cmd_bust:1;
    uint8_t odr_ovwr:1;
    uint8_t avg_ovwr:1;
    uint8_t pwr_mode_is_normal:1;
    uint8_t cmd_is_illegal:1;
    uint8_t :3;
};


//0x08
struct R8_PmuCmdStatus1:public Reg8<>{
    DataRate pmu_odr_s:4;
    AverageTimes pmu_avg_s:2;
    uint8_t :2;
};


//0x09
struct R8_I3cErr:public Reg8<>{
    uint8_t i3c_error_0:1;
    uint8_t :2;
    uint8_t i3c_error_3:1;
    uint8_t :4;
};

//0x0a
struct R8_I2cWdtSet:public Reg8<>{
    uint8_t i2c_wdt_en:1;
    uint8_t i2c_wdt_sel:1;
    uint8_t :6;
};


};
}