#pragma once

#include "sys/core/platform.h"

template<arithmetic T, size_t N_X, size_t N_Y>
class AdaptiveEKF {
    using MatrixXX = Matrix_t<T, N_X, N_X>;
    using MatrixYX = Matrix_t<T, N_Y, N_X>;
    using MatrixXY = Matrix_t<T, N_X, N_Y>;
    using MatrixYY = Matrix_t<T, N_Y, N_Y>;
    using VectorX = Matrix_t<T, N_X, 1>;
    using VectorY = Matrix_t<T, N_Y, 1>;

public:
    explicit AdaptiveEKF(const VectorX &X0 = VectorX::Zero())
            : Xe(X0), P(MatrixXX::Identity()), Q(MatrixXX::Identity()), R(MatrixYY::Identity()) {}


    template<typename Func>
    VectorX predict(Func &&func) {
        Jet_t<T, N_X> Xe_auto_jet[N_X];
        for (int i = 0; i < N_X; i++) {
            Xe_auto_jet[i].a = Xe[i];
            Xe_auto_jet[i].v[i] = 1;
        }
        Jet_t<T, N_X> Xp_auto_jet[N_X];
        func(Xe_auto_jet, Xp_auto_jet);
        for (int i = 0; i < N_X; i++) {
            Xp[i] = Xp_auto_jet[i].a;
            F.block(i, 0, 1, N_X) = Xp_auto_jet[i].v.transpose();
        }
        P = F * P * F.transpose() + Q;
        return Xp;
    }


    template<typename Func>
    VectorX update(Func &&func, const VectorY &Y) {
        Jet_t<T, N_X> Xp_auto_jet[N_X];
        for (size_t i = 0; i < N_X; i++) {
            Xp_auto_jet[i].a = Xp[i];
            Xp_auto_jet[i].v[i] = 1;
        }
        Jet_t<T, N_X> Yp_auto_jet[N_Y];
        func(Xp_auto_jet, Yp_auto_jet);
        for (size_t i = 0; i < N_Y; i++) {
            Yp[i] = Yp_auto_jet[i].a;
            H.block(i, 0, 1, N_X) = Yp_auto_jet[i].v.transpose();
        }
        K = P * H.transpose() * (H * P * H.transpose() + R).inverse();
        Xe = Xp + K * (Y - Yp);
        P = (MatrixXX::Identity() - K * H) * P;
        return Xe;
    }


    VectorX Xe;     // 估计状态变量
    VectorX Xp;     // 预测状态变量
    MatrixXX F;     // 预测雅克比
    MatrixYX H;     // 观测雅克比
    MatrixXX P;     // 状态协方差     *
    MatrixXX Q;     // 预测过程协方差  *   -
    MatrixYY R;     // 观测过程协方差  *   
    MatrixXY K;     // 卡尔曼增益
    VectorY Yp;     // 预测观测量
};

constexpr int Z_N = 2, X_N = 3;

template <arithmetic T>
struct Predict{
    void operator () (const T x0[X_N], T x1[X_N])
    {
        x1[0] = x0[2] * (x0[1] + delta_x) * (x0[1] + delta_x);
        x1[1] = x0[1] + delta_x;
        x1[2] = x0[2];
    }

    scexpr T delta_x = 1;
};



template <typename T>
struct Measure{
    void operator () (const T x0[X_N], T z0[Z_N])
    {
        z0[0] = x0[0];
        z0[1] = z0[1];
    }
};
