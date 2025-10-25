#pragma once

#include <optional>

#include "core/io/regs.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"


namespace ymd::drivers{

struct BMP390_Prelude{
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


struct BMP390_Regset:public BMP390_Prelude{

//0x00
struct R8_ChipId:public Reg8<>{
    static constexpr uint8_t KEY = 0x33;
    uint8_t chip_id_nvm:4;
    uint8_t chip_id_fixed:4;
};

//0x01
struct R8_RevId:public Reg8<>{
    static constexpr uint8_t KEY = 0x33;
    uint8_t rev_id_minor:4;
    uint8_t rev_id_major:4;
};

//0x02
struct R8_Err:public Reg8<>{
    uint8_t fatal_err:1;
    uint8_t cmd_err:1;
    uint8_t conf_err:1;
    uint8_t :5;
};


//0x03
struct R8_Status:public Reg8<>{
    uint8_t :4;
    uint8_t cmd_rdy:1;
    uint8_t drdy_press:1;
    uint8_t drdy_temp:1;
    uint8_t :1;
};


//0x10
struct R8_Event:public Reg8<>{
    uint8_t por_detected:1;
    uint8_t ift_act_pt:1;
    uint8_t :6;
};

//0x11
struct R8_IntStatus:public Reg8<>{
    uint8_t fwm_int:1;
    uint8_t ffull_int:1;
    uint8_t :1;
    uint8_t drdy:1;
    uint8_t :4;
};

//0x17
struct R8_FifoConfig2:public Reg8<>{
    uint8_t fifo_mode:1;
    uint8_t fifo_stop_on_full:1;
    uint8_t fifo_time_en:1;
    uint8_t fifo_press_en:1;
    uint8_t fifo_temp_en:1;
    uint8_t :3;
};

//0x18
struct R8_FifoConfig2:public Reg8<>{
    uint8_t fifo_subsampling:3;
    uint8_t data_select:2;
    uint8_t :3;
};

//0x19
struct R8_IntCtrl:public Reg8<>{
    uint8_t int_od:1;
    uint8_t int_level:1;
    uint8_t int_latch:1;
    uint8_t fwtm_en:1;
    uint8_t ffull_en:1;
    uint8_t int_ds:1;
    uint8_t drdy_en:1;
    uint8_t :1;
};

//0x1A
struct R8_IfConf:public Reg8<>{
    uint8_t spi3:1;
    uint8_t i2c_wdt_en:1;
    uint8_t i2c_wdt_sel:1;
    uint8_t :5;
};

//0x1b
struct R8_PwrCtrl:public Reg8<>{
    uint8_t press_en:1;
    uint8_t temp_en:1;
    uint8_t :2;
    uint8_t mode:2;
    uint8_t :2;
};

//0x1c
struct R8_Osr:public Reg8<>{
    uint8_t osr_p:3;
    uint8_t osr_t:3;
    uint8_t :2;
};

//0x1d
struct R8_Osr:public Reg8<>{
    uint8_t odr_sel:5;
    uint8_t :3;
};

//0x1f
struct R8_Config:public Reg8<>{
    uint8_t short_in:1;
    uint8_t iir_filter:3;
    uint8_t iir_filter:4;
};


};
}