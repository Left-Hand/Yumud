#pragma once

#include "core/io/regs.hpp"

#include "hal/bus/dvp/dvp.hpp"
#include "primitive/image/image.hpp"
#include "core/utils/Errno.hpp"
#include "hal/bus/sccb/sccb_drv.hpp"

namespace ymd::drivers{

class OV2640{
public:
    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x5c);
    static constexpr Vec2i CAMERA_SIZE = {160, 120};
    explicit OV2640(hal::SccbDrv & sccb_drv):
        sccb_drv_(sccb_drv){;}
    explicit OV2640(hal::SccbDrv && sccb_drv):
        sccb_drv_(std::move(sccb_drv)){;}
    explicit OV2640(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        OV2640(hal::SccbDrv{i2c, addr}){;}

    IResult<> init();

    IResult<> validate();

private:

    hal::SccbDrv sccb_drv_;
};


}