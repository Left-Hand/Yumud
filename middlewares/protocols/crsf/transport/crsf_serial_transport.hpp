#pragma once

#include <cstdint>

namespace ymd::crsf::transport{


static constexpr uint32_t STANDERD_BUADRATE = 416666;
static constexpr uint32_t BETAFLIGHT_BUADRATE = 420000;
static constexpr uint32_t INAV_BUADRATE = 420000;
static constexpr uint32_t EXPRESS_LRS_BUADRATE = 420000;

enum class DefaultBaudrate:uint32_t{
    Standerd = STANDERD_BUADRATE,
    Betaflight = BETAFLIGHT_BUADRATE,
    Inav = INAV_BUADRATE,
    ExpressLrs = EXPRESS_LRS_BUADRATE
};

}