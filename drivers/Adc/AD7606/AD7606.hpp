#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"

#include "primitive/analog_channel.hpp"

#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{

class AD7606_Prelude{

};

class AD7606_Phy final: public AD7606_Prelude{
};

class AD7606 final: public AD7606_Prelude{
private:
    AD7606_Phy phy_;
};


}