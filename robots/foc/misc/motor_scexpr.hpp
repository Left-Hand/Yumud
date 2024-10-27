#pragma once

#include "sys/math/real.hpp"


// scexpr uint32_t foc_freq = 32768;
// scexpr uint32_t chopper_freq = 32768;
// scexpr uint32_t foc_freq = 8192 * 4;
scexpr uint32_t foc_freq = 32768;
// scexpr uint32_t foc_freq = 38000;
// scexpr uint32_t foc_freq = 18000;
// scexpr uint32_t foc_freq = 15000;
// scexpr uint32_t foc_freq = 12000;
scexpr uint32_t chopper_freq = 32768;


scexpr real_t pi_4 = real_t(PI/4);
scexpr real_t pi_2 = real_t(PI/2);
scexpr real_t pi = real_t(PI);
scexpr real_t tau = real_t(TAU);

scexpr int poles = 50;
scexpr real_t inv_poles = real_t(1) / poles;

scexpr uint32_t foc_period_micros = 1000000 / foc_freq;