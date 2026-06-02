#pragma once

#include <cstdint>
#include "core/utils/bits/bitfield_proxy.hpp"

namespace ymd::robots::briter{


enum class [[nodiscard]] FaultCode:uint8_t{
    None = 0,
    OverVoltage = 1,
};

}