#pragma once

#include "core/math/realmath.hpp"
#include "dsp/siggen/noise/LCGNoiseSiggen.hpp"

#include "types.hpp"
#include "core/clock/time.hpp"
#include "types/vectors/quat.hpp"

static constexpr auto vec3_compMax(auto v) {return MAX(v.x,v.y,v.z);}
static constexpr auto vec3_compMin(auto v) {return MIN(v.x,v.y,v.z);}

static constexpr auto eye     = Vec3<real_t>(0.0_r, 1.0_r, 3.5_r);

static constexpr auto lightColor = RGB<iq16>{80, 80, 80};

static constexpr auto bbmin = Vec3<real_t>(-1, 0, -1);
static constexpr auto bbmax = Vec3<real_t>(1, 2, 1);


// static constexpr size_t LCD_W = 240;
// static constexpr size_t LCD_H = 135;

static constexpr size_t LCD_W = 120;
static constexpr size_t LCD_H = 120;


// static constexpr size_t LCD_W = 320;
// static constexpr size_t LCD_H = 170;

// static constexpr size_t LCD_W = 160;
// static constexpr size_t LCD_H = 80;

static constexpr real_t INV_LCD_W = real_t(1) / LCD_W;
static constexpr real_t INV_LCD_H = real_t(1) / LCD_H;

static constexpr size_t max_depth = 2;
static constexpr size_t spp  = 1;
static constexpr real_t inv_spp  = 1.0_r/spp;


static constexpr real_t INV_PI       = 0.318310_r;
static constexpr real_t EPSILON      = 0.001_r;
static constexpr real_t light_area   = 0.1893_r;
static constexpr uint alpha       = 45;

