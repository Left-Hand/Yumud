#pragma once

#include <cmath>
// 作者：诺伊斯
// 链接：https://zhuanlan.zhihu.com/p/1900216249989968191
// 来源：知乎
// 著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

static constexpr float d1 = 0.5; // 一些随机扰动项，通常用来确保稳定性，最佳值为2
static constexpr float K1 = 3 * d1;  // K1 应大于 2*d1，最佳值为5
static constexpr float K2 = 5 * K1 * (5 * d1 * K1 + 4 * d1 * d1) / (2 * (K1 - 2 * d1));  // 选择 k2 值
static constexpr float pwm_freq = 20000;
void sta_smo_estimates(float *i_a, float *i_b, float u_a, float u_b, float *e_a, float *e_b) {
    static float e_a_flt , e_b_flt , phase_var  ,  speed_var ,phase_var ;
    float error_a = *i_a - *e_a;
    float error_b = *i_b - *e_b;

    float dt = 1.0 / pwm_freq ;

    if (std::abs(*e_a) > 1.0) {
         K1++; // Increase K1 
    } else {
        K1 = K1 ;
    }
    
    if (fabs(*e_b) > 1.0) {
        k2++; // Increase K2 
    } else {
        k2 = K2 ;
    }

    *i_a = *i_a - (R * dt / L) * (*i_a) + (dt / L) * (K1 * sqrt(fabs(*i_a)) * sign(*i_a) - k2 * sign(error_a));
    *i_b = *i_b - (R * dt / L) * (*i_b) + (dt / L) * (K1 * sqrt(fabs(*i_b)) * sign(*i_b) - k2 * sign(error_b));


    *e_a = -K1 * sqrt(fabs(*i_a)) * sign(*i_a);// + f * k2 * (sign(*i_a) * dt + *e_a);
    *e_b = -K1 * sqrt(fabs(*i_b)) * sign(*i_b);// + f * k2 * (sign(*i_b) * dt + *e_b);

    *e_a += f * k2 * (sign(*i_a) * dt );
    *e_b += f * k2 * (sign(*i_b) * dt );

    utils_truncate_number(e_a , 5 , -5);
    utils_truncate_number(e_b , 5 , -5);

    //*theta = atan(-e_a / e_b);  // Rotor position
    //*omega = sqrt(e_a * e_a + e_b * e_b) / dt ;  // Rotor speed

    UTILS_LP_FAST(e_a_flt , *e_a  , 0.2);
    UTILS_LP_FAST(e_b_flt , *e_b  , 0.2);

    static float *phase = utils_fast_atan2(e_a_flt  ,e_b_flt);
    float delta_theta = phase - *phase_var;

    utils_norm_angle_rad(&delta_theta);
    phase_var += (*speed_var + pll_kp * delta_theta) * dt;

    utils_norm_angle_rad((float*)phase_var);
    speed_var +=  pll_ki * delta_theta * dt;
}