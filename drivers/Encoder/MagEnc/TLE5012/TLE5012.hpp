#pragma once

#include "core/io/regs.hpp"
#include "drivers/Encoder/MagEncoder.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{


class TLE502 final: 
    public MagEncoderIntf, 
    public KTH7823_Regs{
    
};


}