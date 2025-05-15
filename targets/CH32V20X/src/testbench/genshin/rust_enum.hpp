#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"


#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/wireless/Radio/LT8960/LT8960L.hpp"
#include "src/testbench/algo/utils.hpp"

#include "digipw/SVPWM/svpwm3.hpp"

using namespace ymd;

namespace ymd{

enum class Shape{
    Circle,
    Square
};


}