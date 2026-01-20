#pragma once

#include "bno055_prelude.hpp"

namespace ymd::drivers{


class BNO055:
    public BNO055_Prelude{
    

private:
    BNO055_Regs regs_ = {};
};
}