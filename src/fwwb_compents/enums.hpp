#ifndef __FWWB_ENUMS_HPP__

#define __FWWB_ENUMS_HPP__

namespace FWWB{

enum class CanCommand:uint8_t{
    INACTIVE = 0,
    ACTIVE,
    HP,
    MP,
    WEIGHT,

    RST = 0x70,
    POWER_ON = 0x7e,
    OUTBOUND = 0x7f
};
};


#endif