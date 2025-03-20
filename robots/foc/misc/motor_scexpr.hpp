#pragma once

#include "core/math/real.hpp"


namespace ymd::foc{
// scexpr uint32_t foc_freq = 32768;
// scexpr uint32_t chopper_freq = 32768;
// scexpr uint32_t foc_freq = 8192 * 4;
scexpr uint32_t foc_freq = 32768;
// scexpr uint32_t foc_freq = 38000;
// scexpr uint32_t foc_freq = 18000;
// scexpr uint32_t foc_freq = 15000;
// scexpr uint32_t foc_freq = 12000;
scexpr uint32_t chopper_freq = 32768;


scexpr int poles = 50;
scexpr real_t inv_poles = real_t(1) / poles;

scexpr uint32_t foc_period_micros = 1000000 / foc_freq;

}