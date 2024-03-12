#ifndef __KALMAN_FILTER_HPP__

#define __KALMAN_FILTER_HPP__

#include "real.hpp"

template<typename T>
class KalmanFilter_t{

private:
    T r;
    T q;

    T x_last;
    T p_last;

    T p_mid;
    T kg;

    bool init = false;

public:
    template<typename U>
    KalmanFilter_t(const U & _r,const U & _q ):r(static_cast<T>(_r)), q(static_cast<T>(_q)){;}

    template<typename U>
    const T update(const U & x);

    template<typename U>
    const T predict(const U & x);
};

#include "KalmanFilter.tpp"

typedef KalmanFilter_t<real_t> KalmanFilter;


// float angle, angle_dot;    //外部需要引用的变量
// //angle_m为角速度角度和gyro_m为测到的角速度
// void Kalman_Filter(float angle_m,float gyro_m)
// {
//     const float Q_angle=0.001, Q_gyro=0.003, R_angle=0.5, dt=0.005;
//     //注意：dt的取值为kalman滤波器采样时间;        
//     static float P[2][2] = { { 1, 0 },{ 0, 1 } };                           
//     static float Pdot[4] ={0,0,0,0};
//     static const char C_0 = 1;
//     static float q_bias, angle_err, PCt_0, PCt_1, E, K_0, K_1, t_0, t_1;
//     angle+=(gyro_m-q_bias) * dt;
    
//     Pdot[0]=Q_angle - P[0][1] - P[1][0];
//     Pdot[1]=- P[1][1];
//     Pdot[2]=- P[1][1];
//     Pdot[3]=Q_gyro;
    
//     P[0][0] += Pdot[0] * dt;
//     P[0][1] += Pdot[1] * dt;
//     P[1][0] += Pdot[2] * dt;
//     P[1][1] += Pdot[3] * dt;
    
//     angle_err = angle_m - angle;
    
//     PCt_0 = C_0 * P[0][0];
//     PCt_1 = C_0 * P[1][0];
    
//     E = R_angle + C_0 * PCt_0;
    
//     K_0 = PCt_0 / E;
//     K_1 = PCt_1 / E;
    
//     t_0 = PCt_0;
//     t_1 = C_0 * P[0][1];

//     P[0][0] -= K_0 * t_0;
//     P[0][1] -= K_0 * t_1;
//     P[1][0] -= K_1 * t_0;
//     P[1][1] -= K_1 * t_1;
    
//     angle  += K_0 * angle_err;//最终融合角度
//     q_bias += K_1 * angle_err;
//     angle_dot = gyro_m-q_bias;//角速度
// }


#endif
