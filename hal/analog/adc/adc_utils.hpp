#pragma once

#include "adc_primitive.hpp"

namespace ymd::hal{
namespace adc_details{
    void install_pin(const AdcChannelSelection channel, const Enable en);
    real_t read_temp();
}

}