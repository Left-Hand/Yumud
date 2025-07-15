//这个驱动还未完成

#pragma once

#include "core/io/regs.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

class MPR121{
protected:
    hal::I2cDrv i2c_drv_;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x5A >> 1);
    static constexpr size_t MAX_CHANNELS = 12;


    struct R16_Threshold:public Reg16<>{
        uint8_t press;
        uint8_t release;
    };

    struct R16_FiltteredData:public Reg16<>{
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

    struct R8_ChargeTime:public Reg8<>{
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

    struct R16_TouchStatus:public Reg16<>{
        uint16_t ele:13;
        uint16_t __resv__:2;
        uint16_t ovcf:1;
    };

    struct R16_OORStatus:public Reg16<>{
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
        R16_TouchStatus         touch_status;
        R16_OORStatus           oor_status;
        R16_FiltteredData       filtered_datas[MAX_CHANNELS];
        Misc                    misc;
        Misc                    elep_misc;
    
        uint8_t                 baseline_values[MAX_CHANNELS];
        R16_Threshold           thresholds[MAX_CHANNELS];
        uint8_t                 cdc_conf;
        uint8_t                 cdt_conf;
        uint8_t                 electrode_conf;

        uint8_t                 electrode_current[MAX_CHANNELS];
        R8_ChargeTime           charge_times[MAX_CHANNELS/2];
        GpioCtrl                gpio_ctrl;
        AutoConfig              auto_config;
    } Regs;
    
public: 
    MPR121(hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    MPR121(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    MPR121(Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_{hal::I2cDrv{i2c, addr}}{;}
};

}