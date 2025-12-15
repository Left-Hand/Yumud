#pragma once

#include "adc_primitive.hpp"
#include "core/constants/enums.hpp"

namespace ymd::hal{
namespace adc::details{
void install_pin(const AdcChannelSelection sel);
}

}