#pragma once

#include "core/io/regs.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

class MPR121{
protected:
    hal::I2cDrv i2c_drv_;

    static constexpr uint8_t default_i2c_addr = 0x5A;
    static constexpr uint8_t channels = 12;


    struct Threshold:public Reg16<>{
        uint8_t press;
        uint8_t release;
    };

    struct FiltteredData:public Reg16<>{
        uint16_t value:10;
        uint16_t __resv__:6;
    };

    struct GpioCtrl{
        uint8_t ctrl1;
        uint8_t ctrl2;
        uint8_t data;
        uint8_t direction;
        uint8_t enable;
        uint8_t set;
        uint8_t clr;
        uint8_t toggle;
    };

    struct ChargeTime:public Reg8<>{
        uint8_t cdt_a:3;
        uint8_t __resv1__:1;
        uint8_t cdt_b:3;
        uint8_t __resv2__:1;
    };

    struct AutoConfig{
        uint8_t ctrl1;
        uint8_t ctrl2;
        uint8_t usl;
        uint8_t lsl;
        uint8_t target_level;
    };

    struct TouchStatus:public Reg16<>{
        uint16_t ele:13;
        uint16_t __resv__:2;
        uint16_t ovcf:1;
    };

    struct OORStatus:public Reg16<>{
        uint16_t ele:13;
        uint16_t __resv__:1;
        uint16_t arff:1;
        uint16_t acff:1;
    };


    struct Misc{
        uint8_t mhd_rising;
        uint8_t nhd_rising;
        uint8_t ncl_rising;
        uint8_t fdl_rising;
        uint8_t mhd_falling;
        uint8_t nhd_falling;
        uint8_t ncl_falling;
        uint8_t fdl_falling;
        uint8_t nhd_touched;
        uint8_t ncl_touched;
        uint8_t fdl_touched;
    };

    struct {
        TouchStatus touch_status;
        OORStatus oor_status;
        FiltteredData filtered_datas[channels];
        Misc misc;
        Misc elep_misc;
    
        uint8_t bassline_values[channels];
        Threshold thresholds[channels];
        uint8_t cdc_conf;
        uint8_t cdt_conf;
        uint8_t electrode_conf;

        uint8_t electrode_current[channels];
        ChargeTime charge_times[channels/2];
        GpioCtrl gpio_ctrl;
        AutoConfig auto_config;
    } Regs;
    
public: 
    MPR121(hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    MPR121(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    MPR121(hal::I2c & i2c, const uint8_t i2c_addr):i2c_drv_{hal::I2cDrv{i2c, default_i2c_addr}}{;}
};

}