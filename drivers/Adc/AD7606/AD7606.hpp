#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"



#include "hal/conn/spi/spidrv.hpp"


namespace ymd::drivers::ad7606{



class AD7606_Transport final{
};

class AD7606 final{
private:
    AD7606_Transport transport_;
};


}