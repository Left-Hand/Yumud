#pragma once

#include "Robot.hpp"

using Sys::t;

using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::foc;


namespace gxm{


using Vector2 = Vector2_t<real_t>;



void initDisplayer(ST7789 & tftDisplayer);

Ray canvas_transform(const Ray & ray);

void draw_curve(PainterConcept & painter, const Curve & curve);

void print_curve(OutputStream & logger, const Curve & curve);

void draw_turtle(PainterConcept & painter, const Ray & ray);

void test_servo(RadianServo & servo, std::function<real_t(real_t)> && func);

void test_joint(JointLR & joint, std::function<real_t(real_t)> && func);

String getline(InputStream & logger);

struct JointConfig{
    real_t max_rad_delta;
    real_t left_basis_radian;
    real_t right_basis_radian;
    real_t z_basis_radian;
};



}