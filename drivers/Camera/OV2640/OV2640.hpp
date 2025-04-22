#pragma once

#include "core/io/regs.hpp"
#include "drivers/Camera/Camera.hpp" 

namespace ymd::drivers{

class OV2640:public CameraWithSccb<RGB565>{

public:
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x5c);
    scexpr Vector2i CAMERA_SIZE = {160, 120};
public:
    OV2640(hal::SccbDrv & sccb_drv):ImageBasics(CAMERA_SIZE), CameraWithSccb<RGB565>(sccb_drv, CAMERA_SIZE){;}
    OV2640(hal::SccbDrv && sccb_drv):ImageBasics(CAMERA_SIZE), CameraWithSccb<RGB565>(sccb_drv, CAMERA_SIZE){;}
    OV2640(hal::I2c & i2c):
        ImageBasics(CAMERA_SIZE), 
        CameraWithSccb<RGB565>(hal::SccbDrv(i2c, DEFAULT_I2C_ADDR), CAMERA_SIZE){;}

    bool init();

    bool verify();
};


}