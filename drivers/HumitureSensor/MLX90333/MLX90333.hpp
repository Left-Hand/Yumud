#pragma once


#include <tuple>

#include "core/io/regs.hpp"
#include "core/math/real.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"



namespace ymd::drivers{


struct MLX90333_Prelude{

};

struct MLX90333_Regs:public MLX90333_Prelude{

};

class MLX90333_Phy final:public MLX90333_Prelude{

};

class MLX90333 final: public MLX90333_Regs{

};


}