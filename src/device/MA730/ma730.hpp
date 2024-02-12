#ifndef __MA730_HPP__
#define __MA730_HPP__

#include "device/device_defs.h"

class MA730{
protected:
    BusDrv & bus_drv;

    // struct {

    // }
    void writeReg()
public:
    MA730();
    ~MA730();

};

#endif