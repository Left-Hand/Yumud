#pragma once

#include "i2c.hpp"

namespace ymd::hal{

#ifdef I2C1_PRESENT
extern I2c i2c1;
#endif

#ifdef I2C2_PRESENT
extern I2c i2c2;
#endif

}
