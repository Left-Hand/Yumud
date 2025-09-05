#pragma once

#include "core/platform.hpp"

template<arithmetic T>
class KalmanFilter{

public:
    KalmanFilter(
        const T _r,
        const T _q
    ):
        r(static_cast<T>(_r)), 
        q(static_cast<T>(_q)){;}

    const T update(const arithmetic auto x);

    const T predict(const arithmetic auto x);

    void reset(){
        p_last = 0;
        inited = false;
    }


private:
    T r;
    T q;

    T x_last;
    T p_last;

    T p_mid;
    T kg;

    bool inited = false;
};

template<arithmetic T>
const T KalmanFilter<T>::update(const arithmetic auto _x) {
    T x = static_cast<T>(_x);
    if (!inited) {
        x_last = x;
        p_last = T(0);
        inited = true;
    }else {
        x_last = predict(x);
        p_last = (1-kg)*p_mid;
    }
    
    return x_last;
}

template<arithmetic T>
const T KalmanFilter<T>::predict(const arithmetic auto _x) {
    T x = static_cast<T>(_x);
    p_mid = p_last + q;
    kg = p_mid / (p_mid + r);
    return (x_last + kg * (x - x_last));
}


#if 0


void Kalman_Filter_X(real_t Accel,real_t Gyro)		{
    static constexpr real_t Q_angle=0.001_r;// 过程噪声的协方差
	static constexpr real_t Q_gyro=0.003_r;//0.003 过程噪声的协方差 过程噪声的协方差为一个一行两列矩阵
	static constexpr real_t R_angle=0.5_r;// 测量噪声的协方差 既测量偏差
	static constexpr real_t dt=0.01_r;//                 
	static constexpr real_t  C_0 = 1;
	static real_t Q_bias;
	static real_t PP[2][2] = { { 1, 0 },{ 0, 1 } };
    static real_t Angle_Balance_X;
	Angle_Balance_X+=(Gyro - Q_bias) * dt; //先验估计

	PP[0][0] += Q_angle - PP[0][1] - PP[1][0] * dt;   // Pk-先验估计误差协方差微分的积分
	PP[0][1] += -PP[1][1] * dt;   // =先验估计误差协方差
	PP[1][0] += -PP[1][1] * dt;
	PP[1][1] += Q_gyro * dt;
		
	const real_t Angle_err = Accel - Angle_Balance_X;	//zk-先验估计
	
	const real_t PCt_0 = C_0 * PP[0][0];
	const real_t PCt_1 = C_0 * PP[1][0];
	
	const real_t E = R_angle + C_0 * PCt_0;
	const real_t E_inv = 1 / E;
	const real_t K_0 = PCt_0 * E_inv;
	const real_t K_1 = PCt_1 * E_inv;
	
	const real_t t_0 = PCt_0;
	const real_t t_1 = C_0 * PP[0][1];

	PP[0][0] -= K_0 * t_0;		 //后验估计误差协方差
	PP[0][1] -= K_0 * t_1;
	PP[1][0] -= K_1 * t_0;
	PP[1][1] -= K_1 * t_1;
		
	Angle_Balance_X	+= K_0 * Angle_err;	 //后验估计
	Q_bias	+= K_1 * Angle_err;	 //后验估计
	//angle_dot   = Gyro - Q_bias;	 //输出值(后验估计)的微分=角速度
}


#endif