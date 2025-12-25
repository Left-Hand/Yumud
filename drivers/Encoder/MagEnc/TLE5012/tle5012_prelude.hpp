#pragma once

#include "core/io/regs.hpp"
#include "drivers/Encoder/MagEncoder.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct TLE5012_Prelude{

};

struct TLE5012_Regset final: public TLE5012_Prelude{

};


class TLE5012_Transport final: public TLE5012_Prelude{

};

class TLE5012 final: 
    public MagEncoderIntf{
    

private:
    TLE5012_Regset regs_;
};


}