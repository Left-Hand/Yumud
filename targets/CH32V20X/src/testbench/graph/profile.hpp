#pragma once

#include "utils.hpp"


static constexpr auto eye     = Vector3_t(0.0f, 1.0f, 3.5f);
static constexpr auto center  = Vector3_t(0.0f, 1.0f, 0.0f);
static constexpr auto up      = Vector3_t(0.0f, 1.0f, 0.0f);
static constexpr mat4_t view = lookat(eye, center, up);;

static constexpr auto view_x = Vector3_t(view.m[0][0], view.m[0][1], view.m[0][2]);
static constexpr auto view_y = Vector3_t(view.m[1][0], view.m[1][1], view.m[1][2]);
static constexpr auto view_z = Vector3_t(view.m[2][0], view.m[2][1], view.m[2][2]);

static constexpr auto lightColor = Vector3_t<float>::from_ones(200);

static constexpr auto bbmin = Vector3_t<float>(-1, 0, -1);
static constexpr auto bbmax = Vector3_t<float>(1, 2, 1);


#define LCD_W 160
#define LCD_H 80

#define alpha 45

#define spp 1
#define max_depth 2


#define INV_PI 0.318310f
#define EPSILON 0.001f
