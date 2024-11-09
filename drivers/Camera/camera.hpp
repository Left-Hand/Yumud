#pragma once

#include "hal/bus/dvp/dvp.hpp"
#include "SccbDrv.hpp"
#include "types/image/image.hpp"
#include "types/rgb.h"

namespace yumud{


template<typename ColorType>
class CameraWithSccb:public Camera<ColorType>{
public:
    using Vector2 = ImageBasics::Vector2;
    using Vector2i = ImageBasics::Vector2i;
protected:
    SccbDrv sccb_drv_;
    CameraWithSccb(const SccbDrv & sccb_drv, const Vector2i & _size):ImageBasics(_size), Camera<ColorType>(_size), sccb_drv_(sccb_drv){;}
};

};