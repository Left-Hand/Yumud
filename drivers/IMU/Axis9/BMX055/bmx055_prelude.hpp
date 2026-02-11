#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"

namespace ymd::drivers::bmx055{


struct [[nodiscard]] BMX055_Prelude{

};

struct [[nodiscard]] BMX055_Regs : public BMX055_Prelude{ 
};
}