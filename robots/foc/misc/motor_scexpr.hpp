#pragma once

#include "core/math/real.hpp"


namespace ymd::foc{
// static constexpr uint32_t foc_freq = 32768;
// static constexpr uint32_t chopper_freq = 32768;
// static constexpr uint32_t foc_freq = 8192 * 4;
static constexpr uint32_t foc_freq = 32768;
// static constexpr uint32_t foc_freq = 38000;
// static constexpr uint32_t foc_freq = 18000;
// static constexpr uint32_t foc_freq = 15000;
// static constexpr uint32_t foc_freq = 12000;
static constexpr uint32_t chopper_freq = 32768;


static constexpr int poles = 50;
static constexpr real_t inv_poles = real_t(1) / poles;

static constexpr uint32_t foc_period_micros = 1000000 / foc_freq;

}