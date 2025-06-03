#pragma once

#include "core/io/regs.hpp"
#include "drivers/Encoder/MagEncoder.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct TLE5012_Collections{

};

struct TLE5012_Regs final: public TLE5012_Collections{

};


class TLE5012_Phy final: public TLE5012_Collections{

};

class TLE5012 final: 
    public MagEncoderIntf, 
    public TLE5012_Regs{
    
};


}