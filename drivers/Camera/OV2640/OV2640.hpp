#pragma once

#include "drivers/device_defs.h"
#include "drivers/Camera/Camera.hpp" 

namespace ymd::drivers{

class OV2640:public CameraWithSccb<RGB565>{

public:
    scexpr uint8_t default_addr = 0x5c << 1;
    scexpr Vector2i camera_size = {160, 120};
public:
    OV2640(hal::SccbDrv & sccb_drv):ImageBasics(camera_size), CameraWithSccb<RGB565>(sccb_drv, camera_size){;}
    OV2640(hal::SccbDrv && sccb_drv):ImageBasics(camera_size), CameraWithSccb<RGB565>(sccb_drv, camera_size){;}
    OV2640(hal::I2c & _i2c):ImageBasics(camera_size), CameraWithSccb<RGB565>(hal::SccbDrv(_i2c, default_addr), camera_size){;}

    bool init();

    bool verify();
};


}