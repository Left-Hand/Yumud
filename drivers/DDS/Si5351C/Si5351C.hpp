#pragma once

#include "drivers/device_defs.h"

namespace ymd::drivers{

class Si5351C{
protected:
    hal::I2cDrv _i2c_drv;

    struct DeviceStatusReg:public Reg8<>{
        scexpr uint8_t address = 0x00;

        uint8_t REVID:2;
        uint8_t :2;
        uint8_t LOS:1;
        uint8_t LOL_A:1;
        uint8_t LOL_B:1;
        uint8_t SYS_INIT:1;
    };

    struct InterruptStatusReg:public Reg8<>{
        scexpr uint8_t address = 0x01;

        uint8_t :4;
        uint8_t LOS_STKY:1;
        uint8_t LOL_A_STKY:1;
        uint8_t LOL_B_STKY:1;
        uint8_t SYS_INIT_STKY:1;
    };

    struct InterruptMaskReg:public Reg8<>{
        scexpr uint8_t address = 0x02;

        uint8_t :4;
        uint8_t LOS_MASK:1;
        uint8_t LOL_A_MASK:1;
        uint8_t LOL_B_MASK:1;
        uint8_t SYS_INIT_MASK:1;
    };

    struct OutputEnableCtrlReg:public Reg8<>{
        scexpr uint8_t address = 0x03;

        uint8_t CLK0_OEB:1;
        uint8_t CLK1_OEB:1;
        uint8_t CLK2_OEB:1;
        uint8_t CLK3_OEB:1;
        uint8_t CLK4_OEB:1;
        uint8_t CLK5_OEB:1;
        uint8_t CLK6_OEB:1;
        uint8_t CLK7_OEB:1;
    };

    struct OEBPinEnableCtrlReg:public Reg8<>{
        scexpr uint8_t address = 0x09;

        uint8_t OEB_CLK0:1;
        uint8_t OEB_CLK1:1;
        uint8_t OEB_CLK2:1;
        uint8_t OEB_CLK3:1;
        uint8_t OEB_CLK4:1;
        uint8_t OEB_CLK5:1;
        uint8_t OEB_CLK6:1;
        uint8_t OEB_CLK7:1;
    };

    struct PllInputSourceReg:public Reg8<>{
        scexpr uint8_t address = 15;

        uint8_t :2;
        uint8_t PLLA_SOURCE:1;
        uint8_t PLLB_SOURCE:1;
        uint8_t :4;
    };

    struct CLKCtrlReg:public Reg8<>{
        scexpr uint8_t address = 16;

        uint8_t CLK_IDRV:2;
        uint8_t CLK_SRC:2;
        uint8_t CLK_INV:1;
        uint8_t MS_SRC:1;
        uint8_t MS_INT:1;
        uint8_t CLK_PDN:4;
    };

    struct CLK0CtrlReg:public Reg8<>{
        scexpr uint8_t address = 16;

        uint8_t CLK0_IDRV:2;
        uint8_t CLK0_SRC:2;
        uint8_t CLK0_INV:1;
        uint8_t MS0_SRC:1;
        uint8_t MS0_INT:1;
        uint8_t CLK0_PDN:4;
    };

    struct CLK1CtrlReg:public Reg8<>{
        scexpr uint8_t address = 17;

        uint8_t CLK1_IDRV:2;
        uint8_t CLK1_SRC:2;
        uint8_t CLK1_INV:1;
        uint8_t MS1_SRC:1;
        uint8_t MS1_INT:1;
        uint8_t CLK1_PDN:4;
    };

    struct CLK2CtrlReg:public Reg8<>{
        scexpr uint8_t address = 18;

        uint8_t CLK2_IDRV:2;
        uint8_t CLK2_SRC:2;
        uint8_t CLK2_INV:1;
        uint8_t MS2_SRC:1;
        uint8_t MS2_INT:1;
        uint8_t CLK2_PDN:4;
    };

    struct CLK3_0DisableStateReg:public Reg8<>{
        scexpr uint8_t address = 24;

        uint8_t CLK0_DIS_STATE:2;
        uint8_t CLK1_DIS_STATE:2;
        uint8_t CLK2_DIS_STATE:2;
        uint8_t CLK3_DIS_STATE:2;
    };

    struct CLK7_4DisableStateReg:public Reg8<>{
        scexpr uint8_t address = 25;

        uint8_t CLK4_DIS_STATE:2;
        uint8_t CLK5_DIS_STATE:2;
        uint8_t CLK6_DIS_STATE:2;
        uint8_t CLK7_DIS_STATE:2;
    };

    // struct 
protected:
    void writeReg(const uint8_t address, const uint8_t data);
    void writeMulti(const uint8_t address, const uint8_t * data, const size_t len);
    void readReg(const uint8_t address, uint8_t & data);
    void readMulti(const uint8_t address, uint8_t * data, const size_t len);
public:
    Si5351C(const hal::I2cDrv & i2c_drv):
        _i2c_drv(i2c_drv){};
    Si5351C(hal::I2cDrv && i2c_drv):
        _i2c_drv(std::move(i2c_drv)){};
    
    void init();

};

}