#pragma once

#include "core/utils/reg_base.hpp"
#include "drivers/IMU/IMU.hpp"

namespace ymd::drivers::bmx055{


struct [[nodiscard]] BMX055_Prelude{

};

struct [[nodiscard]] BMX055_Regs : public BMX055_Prelude{ 
};
}