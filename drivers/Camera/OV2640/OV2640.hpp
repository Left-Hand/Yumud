#pragma once

#include "core/io/regs.hpp"
#include "drivers/Camera/Camera.hpp" 

namespace ymd::drivers{

class OV2640:public Camera<RGB565>{

public:
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x5c);
    scexpr Vector2i CAMERA_SIZE = {160, 120};
    OV2640(hal::SccbDrv & sccb_drv):
        ImageBasics(CAMERA_SIZE), 
        Camera<RGB565>(CAMERA_SIZE),
        sccb_drv_(sccb_drv){;}
    OV2640(hal::SccbDrv && sccb_drv):
        ImageBasics(CAMERA_SIZE), 
        Camera<RGB565>(CAMERA_SIZE),
        sccb_drv_(sccb_drv){;}
    OV2640(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        OV2640(hal::SccbDrv{i2c, addr}){;}

    bool init();

    bool validate();

private:

    hal::SccbDrv sccb_drv_;
};


}