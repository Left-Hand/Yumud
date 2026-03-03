#pragma once

/**
 ******************************************************************************
 * @file    robotics.cpp/h
 * @brief   Robotic toolbox on STM32. STM32机器人学库
 * @author  Spoon Guan
 * @ref     [1] SJTU ME385-2, Robotics, Y.Ding
 *          [2] Bruno Siciliano, et al., Robotics: Modelling, Planning and
 *              Control, Springer, 2010.
 *          [3] R.Murry, Z.X.Li, and S.Sastry, A Mathematical Introduction
 *              to Robotic Manipulation, CRC Press, 1994.
 ******************************************************************************
 * Copyright (c) 2023 Team JiaoLong-SJTU
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************
 */

/**
 * SPDX-License-Identifier: Apache-2.0
 *
 * Modified for use in Apache 2.0 project [Yumud]
 * Modifications include:
 * - 性能优化
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "robotics_utils.hpp"

namespace ymd::robots::robotics{

// joint type: R-revolute joint, P-prismatic joint
enum class [[nodiscard]] JointType{
    R = 0,
    P = 1,
} ;

// Denavit–Hartenberg(DH) method
struct [[nodiscard]] DH final{
    // forward kinematic

    // DH parameter
    float theta;
    float d;
    float a;
    float alpha;
    // math::Matrix<float, 4, 4> T;

    math::Matrix<float, 4, 4> fkine(){
        math::Matrix<float, 4, 4> T;
        float ct = math::cos(theta), st = math::sin(theta);  // cosθ, sinθ
        float ca = math::cos(alpha), sa = math::sin(alpha);  // cosα, sinα

        // T =
        // | cθ  -sθcα   sθsα   acθ |
        // | sθ   cθcα  -cθsα   asθ |
        // |  0     sα     cα     d |
        // |  0      0      0     1 |
        T[0][0] = ct;
        T[0][1] = -st * ca;
        T[0][2] = st * sa;
        T[0][3] = a * ct;

        T[1][0] = st;
        T[1][1] = ct * ca;
        T[1][2] = -ct * sa;
        T[1][3] = a * st;

        T[2][0] = 0;
        T[2][1] = sa;
        T[2][2] = ca;
        T[2][3] = d;

        T[3][0] = 0;
        T[3][1] = 0;
        T[3][2] = 0;
        T[3][3] = 1;

        return T;

    }

};

struct [[nodiscard]] Link final{
public:

    math::Matrix<float, 4, 4> T(float q){
        switch(type_){
            case JointType::R:
                if (qmin_ >= qmax_)
                    dh_.theta = q + offset_;
                else
                    dh_.theta = math::clamp(q + offset_, qmin_, qmax_);
                break;
            case JointType::P:
                if (qmin_ >= qmax_)
                    dh_.d = q + offset_;
                else
                    dh_.d = math::clamp(q + offset_, qmin_, qmax_);
        }
        return dh_.fkine();
    }

public:


    // kinematic parameter
    DH dh_;
    float offset_;
    // clamp(qmin,qmax), no clamp if qmin<=qmax
    float qmin_;
    float qmax_;
    // joint type
    JointType type_;
    // dynamic parameter
    float m_;           // mass
    math::Matrix<float, 3, 1> rc_;  // centroid(link coordinate)
    math::Matrix<float, 3, 3> I_;   // inertia tensor(3*3)

};

template <size_t N = 1>
struct [[nodiscard]] Serial_Link final{
public:
    Serial_Link(Link links[N]) {
        for (int i = 0; i < N; i++)
        links_[i] = links[i];
        gravity_ = matrixf::zeros<3, 1>();
        gravity_[2][0] = -9.81f;
    }

    Serial_Link(Link links[N], math::Matrix<float, 3, 1> gravity) {
        for (int i = 0; i < N; i++)
        links_[i] = links[i];
        gravity_ = gravity;
    }

    // forward kinematic: T_n^0
    // param[in] q: joint variable vector
    // param[out] T_n^0
    math::Matrix<float, 4, 4> fkine(math::Matrix<float, N, 1> q) {
        T_ = matrixf::eye<4, 4>();
        for (int iminus1 = 0; iminus1 < N; iminus1++)
            T_ = T_ * links_[iminus1].T(q[iminus1][0]);
        return T_;
    }

    // forward kinematic: T_k^0
    // param[in] q: joint variable vector
    // param[in] k: joint number
    // param[out] T_k^0
    math::Matrix<float, 4, 4> fkine(math::Matrix<float, N, 1> q, uint16_t k) {
        if (k > N)
        k = N;
        math::Matrix<float, 4, 4> T = matrixf::eye<4, 4>();
        for (int iminus1 = 0; iminus1 < k; iminus1++)
        T = T * links_[iminus1].T(q[iminus1][0]);
        return T;
    }

    // T_k^k-1: homogeneous transformation matrix of link k
    // param[in] q: joint variable vector
    // param[in] kminus: joint number k, input k-1
    // param[out] T_k^k-1
    math::Matrix<float, 4, 4> T(math::Matrix<float, N, 1> q, uint16_t kminus1) {
        if (kminus1 >= N)
        kminus1 = N - 1;
        return links_[kminus1].T(q[kminus1][0]);
    }

    // jacobian matrix, J_i = [J_pi;j_oi]
    // param[in] q: joint variable vector
    // param[out] jacobian matix J_6*n
    math::Matrix<float, 6, N> jacob(math::Matrix<float, N, 1> q) {
        math::Matrix<float, 3, 1> p_e = t2p(fkine(q));               // p_e
        math::Matrix<float, 4, 4> T_iminus1 = matrixf::eye<4, 4>();  // T_i-1^0
        math::Matrix<float, 3, 1> z_iminus1;                         // z_i-1^0
        math::Matrix<float, 3, 1> p_iminus1;                         // p_i-1^0
        math::Matrix<float, 3, 1> J_pi;
        math::Matrix<float, 3, 1> J_oi;
        for (int iminus1 = 0; iminus1 < N; iminus1++) {
        // revolute joint: J_pi = z_i-1x(p_e-p_i-1), J_oi = z_i-1
        if (links_[iminus1].type() == R) {
            z_iminus1 = T_iminus1.submatrix<3, 1>(0, 2);
            p_iminus1 = t2p(T_iminus1);
            T_iminus1 = T_iminus1 * links_[iminus1].T(q[iminus1][0]);
            J_pi = vector3f::cross(z_iminus1, p_e - p_iminus1);
            J_oi = z_iminus1;
        }
        // prismatic joint: J_pi = z_i-1, J_oi = 0
        else {
            z_iminus1 = T_iminus1.submatrix<3, 1>(0, 2);
            T_iminus1 = T_iminus1 * links_[iminus1].T(q[iminus1][0]);
            J_pi = z_iminus1;
            J_oi = matrixf::zeros<3, 1>();
        }
        J_[0][iminus1] = J_pi[0][0];
        J_[1][iminus1] = J_pi[1][0];
        J_[2][iminus1] = J_pi[2][0];
        J_[3][iminus1] = J_oi[0][0];
        J_[4][iminus1] = J_oi[1][0];
        J_[5][iminus1] = J_oi[2][0];
        }
        return J_;
    }

    // inverse kinematic, numerical solution(Newton method)
    // param[in] T: homogeneous transformation matrix of end effector
    // param[in] q: initial joint variable vector(q0) for Newton method's
    //              iteration
    // param[in] tol: tolerance of error (norm(error of twist vector))
    // param[in] max_iter: maximum iterations, default 30
    // param[out] q: joint variable vector
    math::Matrix<float, N, 1> ikine(math::Matrix<float, 4, 4> Td,
                        math::Matrix<float, N, 1> q = matrixf::zeros<N, 1>(),
                        float tol = 1e-4f, uint16_t max_iter = 50) {
        math::Matrix<float, 4, 4> T;
        math::Matrix<float, 3, 1> pe, we;
        math::Matrix<float, 6, 1> err, new_err;
        math::Matrix<float, N, 1> dq;
        float step = 1;
        for (int i = 0; i < max_iter; i++) {
        T = fkine(q);
        pe = t2p(Td) - t2p(T);
        // angvec(Td*T^-1), transform angular vector(T->Td) in world coordinate
        we = t2twist(Td * invT(T)).submatrix<3, 1>(3, 0);
        for (int i = 0; i < 3; i++) {
            err[i][0] = pe[i][0];
            err[i + 3][0] = we[i][0];
        }
        if (err.norm() < tol)
            return q;
        // adjust iteration step
        math::Matrix<float, 6, N> J = jacob(q);
        for (int j = 0; j < 5; j++) {
            dq = matrixf::inv(J.trans() * J) * (J.trans() * err) * step;
            if (dq[0][0] == INFINITY)  // J'*J singular
            {
            dq = matrixf::inv(J.trans() * J + 0.1f * matrixf::eye<N, N>()) *
                J.trans() * err * step;
            // SVD<6, N> JTJ_svd(J.trans() * J);
            // dq = JTJ_svd.solve(err) * step * 5e-2f;
            q += dq;
            for (int i = 0; i < N; i++) {
                if (links_[i].type() == R)
                q[i][0] = math::loopLimit(q[i][0], -static_cast<float>(M_PI), static_cast<float>(M_PI));
            }
            break;
            }
            T = fkine(q + dq);
            pe = t2p(Td) - t2p(T);
            we = t2twist(Td * invT(T)).submatrix<3, 1>(3, 0);
            for (int i = 0; i < 3; i++) {
            new_err[i][0] = pe[i][0];
            new_err[i + 3][0] = we[i][0];
            }
            if (new_err.norm() < err.norm()) {
            q += dq;
            for (int i = 0; i < N; i++) {
                if (links_[i].type() == JointType::R) {
                q[i][0] = math::loopLimit(q[i][0], -static_cast<float>(M_PI), static_cast<float>(M_PI));
                }
            }
            break;
            } else {
            step /= 2.0f;
            }
        }
        if (step < 1e-3f)
            return q;
        }
        return q;
    }

    // (Reserved function) inverse kinematic, analytic solution(geometric method)
    using ReservedFunction = std::function<math::Matrix<float, N, 1>(math::Matrix<float, 4, 4> T)>;
    ReservedFunction ikine_analytic;

    // inverse dynamic, Newton-Euler method
    // param[in]  q: joint variable vector
    // param[in]  qv: dq/dt
    // param[in]  qa: d^2q/dt^2
    // param[in]  he: load on end effector [f;μ], default 0
    math::Matrix<float, N, 1> rne(
        math::Matrix<float, N, 1> q,
        math::Matrix<float, N, 1> qv = matrixf::zeros<N, 1>(),
        math::Matrix<float, N, 1> qa = matrixf::zeros<N, 1>(),
        math::Matrix<float, 6, 1> he = matrixf::zeros<6, 1>()
    ) {
        // forward propagation
        // record each links' motion state in matrices
        // [ωi] angular velocity
        math::Matrix<float, 3, N + 1> w = matrixf::zeros<3, N + 1>();
        // [βi] angular acceleration
        math::Matrix<float, 3, N + 1> b = matrixf::zeros<3, N + 1>();
        // [pi] position of joint
        math::Matrix<float, 3, N + 1> p = matrixf::zeros<3, N + 1>();
        // [vi] velocity of joint
        math::Matrix<float, 3, N + 1> v = matrixf::zeros<3, N + 1>();
        // [ai] acceleration of joint
        math::Matrix<float, 3, N + 1> a = matrixf::zeros<3, N + 1>();
        // [aci] acceleration of mass center
        math::Matrix<float, 3, N + 1> ac = matrixf::zeros<3, N + 1>();
        // temperary vectors
        math::Matrix<float, 3, 1> w_i, b_i, p_i, v_i, ai, ac_i;
        // i & i-1 coordinate convert to 0 coordinate
        math::Matrix<float, 4, 4> T_0i = matrixf::eye<4, 4>();
        math::Matrix<float, 4, 4> T_0iminus1 = matrixf::eye<4, 4>();
        math::Matrix<float, 3, 3> R_0i = matrixf::eye<3, 3>();
        math::Matrix<float, 3, 3> R_0iminus1 = matrixf::eye<3, 3>();
        // unit vector of z-axis
        math::Matrix<float, 3, 1> ez = matrixf::zeros<3, 1>();
        ez[2][0] = 1;

        for (int i = 1; i <= N; i++) {
            T_0i = T_0i * T(q, i - 1);     // T_i^0
            R_0i = t2r(T_0i);              // R_i^0
            R_0iminus1 = t2r(T_0iminus1);  // R_i-1^0
            // ω_i = ω_i-1+qv_i*R_i-1^0*ez
            w_i = w.col(i - 1) + qv[i - 1][0] * R_0iminus1 * ez;
            // β_i = β_i-1+ω_i-1x(qv_i*R_i-1^0*ez)+qa_i*R_i-1^0*ez
            b_i = b.col(i - 1) +
                    vector3f::cross(w.col(i - 1), qv[i - 1][0] * R_0iminus1 * ez) +
                    qa[i - 1][0] * R_0iminus1 * ez;
            p_i = t2p(T_0i);  // p_i = T_i^0(1:3,4)
            // v_i = v_i-1+ω_ix(p_i-p_i-1)
            v_i = v.col(i - 1) + vector3f::cross(w_i, p_i - p.col(i - 1));
            // a_i = a_i-1+β_ix(p_i-p_i-1)+ω_ix(ω_ix(p_i-p_i-1))
            ai = a.col(i - 1) + vector3f::cross(b_i, p_i - p.col(i - 1)) +
                vector3f::cross(w_i, vector3f::cross(w_i, p_i - p.col(i - 1)));
            // ac_i = a_i+β_ix(R_0^i*rc_i^i)+ω_ix(ω_ix(R_0^i*rc_i^i))
            ac_i =
                ai + vector3f::cross(b_i, R_0i * links_[i - 1].rc()) +
                vector3f::cross(w_i, vector3f::cross(w_i, R_0i * links_[i - 1].rc()));
            for (int row = 0; row < 3; row++) {
                w[row][i] = w_i[row][0];
                b[row][i] = b_i[row][0];
                p[row][i] = p_i[row][0];
                v[row][i] = v_i[row][0];
                a[row][i] = ai[row][0];
                ac[row][i] = ac_i[row][0];
            }
            T_0iminus1 = T_0i;  // T_i-1^0
        }

        // backward propagation
        // record each links' force
        math::Matrix<float, 3, N + 1> f = matrixf::zeros<3, N + 1>();   // joint force
        math::Matrix<float, 3, N + 1> mu = matrixf::zeros<3, N + 1>();  // joint moment
        // temperary vector
        math::Matrix<float, 3, 1> f_iminus1, mu_iminus1;
        // {T,R',P}_i^i-1
        math::Matrix<float, 4, 4> T_iminus1i;
        math::Matrix<float, 3, 3> RT_iminus1i;
        math::Matrix<float, 3, 1> P_iminus1i;
        // I_i-1(in 0 coordinate)
        math::Matrix<float, 3, 3> I_i;
        // joint torque
        math::Matrix<float, N, 1> torq;

        // load on end effector
        for (int row = 0; row < 3; row++) {
            f[row][N] = he.submatrix<3, 1>(0, 0)[row][0];
            mu[row][N] = he.submatrix<3, 1>(3, 0)[row][0];
        }
        for (int i = N; i > 0; i--) {
            T_iminus1i = T(q, i - 1);               // T_i^i-1
            P_iminus1i = t2p(T_iminus1i);           // P_i^i-1
            RT_iminus1i = t2r(T_iminus1i).trans();  // R_i^i-1'
            R_0iminus1 = R_0i * RT_iminus1i;        // R_i-1^0
            // I_i^0 = R_i^0*I_i^i*(R_i^0)'
            I_i = R_0i * links_[i - 1].I() * R_0i.trans();
            // f_i-1 = f_i+m_i*ac_i-m_i*g
            f_iminus1 = f.col(i) + links_[i - 1].m() * ac.col(i) -
                        links_[i - 1].m() * gravity_;
            // μ_i-1 = μ_i+f_ixrc_i-f_i-1xrc_i-1->ci+I_i*b_i+ω_ix(I_i*ω_i)
            mu_iminus1 = mu.col(i) +
                        vector3f::cross(f.col(i), R_0i * links_[i - 1].rc()) -
                        vector3f::cross(f_iminus1, R_0i * (RT_iminus1i * P_iminus1i +
                                                            links_[i - 1].rc())) +
                        I_i * b.col(i) + vector3f::cross(w.col(i), I_i * w.col(i));
            // τ_i = μ_i-1'*(R_i-1^0*ez)
            torq[i - 1][0] = (mu_iminus1.trans() * R_0iminus1 * ez)[0][0];
            for (int row = 0; row < 3; row++) {
                f[row][i - 1] = f_iminus1[row][0];
                mu[row][i - 1] = mu_iminus1[row][0];
            }
            R_0i = R_0iminus1;
        }

        return torq;
    }

private:
    Link links_[N];
    math::Matrix<float, 3, 1> gravity_;

    math::Matrix<float, 4, 4> T_;
    math::Matrix<float, 6, N> J_;
};
}