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


//  https://github.com/SJTU-RoboMaster-Team/Matrix_and_Robotics_on_STM32/blob/main/src/robotics.h

#include "core/math/realmath.hpp"
#include "core/math/matrix/static_matrix.hpp"

namespace ymd::robotics{

/**
 * 将旋转矩阵转换为ZYX欧拉角（偏航-俯仰-滚转）
 * @param R 3x3旋转矩阵
 * @return 包含[yaw, pitch, roll]的3x1向量
 */
static constexpr math::Matrix<float, 3, 1> r2rpy(const math::Matrix<float, 3, 3> & R) {
    return math::Matrix<float, 3, 1>(
        math::atan2(R[1][0], R[0][0]),                                        // yaw
        math::atan2(-R[2][0], math::sqrt(R[2][1] * R[2][1] + R[2][2] * R[2][2])),  // pitch
        math::atan2(R[2][1], R[2][2])                                         // roll
    );
}

/**
 * 将ZYX欧拉角（偏航-俯仰-滚转）转换为旋转矩阵
 * @param rpy 包含[yaw, pitch, roll]的3x1向量
 * @return 3x3旋转矩阵
 */
static constexpr math::Matrix<float, 3, 3> rpy2r(const math::Matrix<float, 3, 1>& rpy) {
    float c[3] = {math::cos(rpy[0][0]), math::cos(rpy[1][0]), math::cos(rpy[2][0])};
    float s[3] = {math::sin(rpy[0][0]), math::sin(rpy[1][0]), math::sin(rpy[2][0])};
    return math::Matrix<float, 3, 3>(
        c[0] * c[1],                       // R11
        c[0] * s[1] * s[2] - s[0] * c[2],  // R12
        c[0] * s[1] * c[2] + s[0] * s[2],  // R13
        s[0] * c[1],                       // R21
        s[0] * s[1] * s[2] + c[0] * c[2],  // R22
        s[0] * s[1] * c[2] - c[0] * s[2],  // R23
        -s[1],                             // R31
        c[1] * s[2],                       // R32
        c[1] * c[2]                        // R33
    );
}

/**
 * 将旋转矩阵转换为轴角表示
 * @param R 3x3旋转矩阵
 * @return 包含[rx, ry, rz, theta]的4x1向量，其中r为单位轴向量，theta为旋转角度
 */
static constexpr math::Matrix<float, 4, 1> r2angvec(const math::Matrix<float, 3, 3>& R) {
    float theta = math::acos(math::clamp<float>(0.5f * (R.trace() - 1), -1.0, 1.0));
    if (theta == 0 || theta == static_cast<float>(M_PI)) {
        return math::Matrix<float, 4, 1>(
            (1 + R[0][0] - R[1][1] - R[2][2]) * 0.25f,  // rx=(1+R11-R22-R33)/4
            (1 - R[0][0] + R[1][1] - R[2][2]) * 0.25f,  // ry=(1-R11+R22-R33)/4
            (1 - R[0][0] - R[1][1] + R[2][2]) * 0.25f,  // rz=(1-R11-R22+R33)/4
            theta   
        );
    }else{
        const auto inv_len = 1.0 / (2 * math::sin(theta));
        return math::Matrix<float, 4, 1>(
            (R[2][1] - R[1][2]) * inv_len,  // rx=(R32-R23)/2sinθ
            (R[0][2] - R[2][0]) * inv_len,  // ry=(R13-R31)/2sinθ
            (R[1][0] - R[0][1]) * inv_len,  // rz=(R21-R12)/2sinθ
            theta                                        // theta
        );

    }

}

/**
 * 将轴角表示转换为旋转矩阵
 * @param angvec 包含[rx, ry, rz, theta]的4x1向量，其中r为旋转轴，theta为旋转角度
 * @return 3x3旋转矩阵
 */
static constexpr math::Matrix<float, 3, 3> angvec2r(const math::Matrix<float, 4, 1>& angvec) {
    float theta = angvec[3][0];
    math::Matrix<float, 3, 1> r = angvec.submatrix<3, 1>(0, 0);
    
    // 单位化旋转轴
    float norm_r = math::sqrt(r[0][0]*r[0][0] + r[1][0]*r[1][0] + r[2][0]*r[2][0]);
    if (norm_r > 1e-6f) {
        float inv_norm = 1.0f / norm_r;
        r[0][0] *= inv_norm;
        r[1][0] *= inv_norm;
        r[2][0] *= inv_norm;
    }
    
    float c = math::cos(theta);
    float s = math::sin(theta);
    float v = 1 - c;
    
    float x = r[0][0];
    float y = r[1][0];
    float z = r[2][0];
    
    float xyv = x * y * v;
    float xzv = x * z * v;
    float yzv = y * z * v;
    
    float xs = x * s;
    float ys = y * s;
    float zs = z * s;
    
    return math::Matrix<float, 3, 3>(
        // 第一行
        x*x*v + c,
        xyv - zs,
        xzv + ys,
        
        // 第二行
        xyv + zs,
        y*y*v + c,
        yzv - xs,
        
        // 第三行
        xzv - ys,
        yzv + xs,
        z*z*v + c
    );
}

/**
 * 将旋转矩阵转换为四元数表示
 * @param R 3x3旋转矩阵
 * @return 包含[q0, q1, q2, q3]的4x1向量，其中q0为实部，[q1,q2,q3]为虚部
 */
static constexpr math::Matrix<float, 4, 1> r2quat(const math::Matrix<float, 3, 3>& R) {
    auto Q = math::Matrix<float, 4, 1>(
        0.5f * math::sqrt(math::clamp<float>(R.trace(), -1, 1) + 1),  // q0=cos(θ/2)
        math::sign(R[2][1] - R[1][2]) * 0.5f *
            math::sqrt(math::clamp<float>(R[0][0] - R[1][1] - R[2][2], -1, 1) + 1),  // q1=rx*sin(θ/2)
        math::sign(R[0][2] - R[2][0]) * 0.5f *
            math::sqrt(math::clamp<float>(-R[0][0] + R[1][1] - R[2][2], -1, 1) +1),  // q2=ry*sin(θ/2)
        math::sign(R[1][0] - R[0][1]) * 0.5f *
            math::sqrt(math::clamp<float>(-R[0][0] - R[1][1] + R[2][2], -1, 1) +1)  // q3=rz*sin(θ/2)
    );

    const auto inv_len = math::inv_mag(Q.at<0, 0>(), Q.at<1, 0>(), Q.at<2, 0>(), Q.at<3, 0>());
    return Q * inv_len;
}

/**
 * 将四元数转换为旋转矩阵
 * @param q 包含[q0, q1, q2, q3]的4x1向量
 * @return 3x3旋转矩阵
 */
static constexpr math::Matrix<float, 3, 3> quat2r(const math::Matrix<float, 4, 1>& q) {
    return math::Matrix<float, 3, 3>(
        1 - 2 * (q[2][0] * q[2][0] + q[3][0] * q[3][0]),  // R11
        2 * (q[1][0] * q[2][0] - q[0][0] * q[3][0]),      // R12
        2 * (q[1][0] * q[3][0] + q[0][0] * q[2][0]),      // R13
        2 * (q[1][0] * q[2][0] + q[0][0] * q[3][0]),      // R21
        1 - 2 * (q[1][0] * q[1][0] + q[3][0] * q[3][0]),  // R22
        2 * (q[2][0] * q[3][0] - q[0][0] * q[1][0]),      // R23
        2 * (q[1][0] * q[3][0] - q[0][0] * q[2][0]),      // R31
        2 * (q[2][0] * q[3][0] + q[0][0] * q[1][0]),      // R32
        1 - 2 * (q[1][0] * q[1][0] + q[2][0] * q[2][0])   // R33
    );
}

/**
 * 将四元数转换为ZYX欧拉角
 * @param q 包含[q0, q1, q2, q3]的4x1向量
 * @return 包含[yaw, pitch, roll]的3x1向量
 */
static constexpr math::Matrix<float, 3, 1> quat2rpy(const math::Matrix<float, 4, 1>& q) {
    return math::Matrix<float, 3, 1>(
        math::atan2(2 * (q[1][0] * q[2][0] + q[0][0] * q[3][0]),
            1 - 2 * (q[2][0] * q[2][0] + q[3][0] * q[3][0])),  // yaw
        math::asin(2 * (q[0][0] * q[2][0] - q[1][0] * q[3][0])),       // pitch
        math::atan2(2 * (q[2][0] * q[3][0] + q[0][0] * q[1][0]),
                1 - 2 * (q[1][0] * q[1][0] + q[2][0] * q[2][0]))  // rol
    );
}

/**
 * 将ZYX欧拉角转换为四元数
 * @param rpy 包含[yaw, pitch, roll]的3x1向量
 * @return 包含[q0, q1, q2, q3]的4x1向量
 */
static constexpr math::Matrix<float, 4, 1> rpy2quat(const math::Matrix<float, 3, 1>& rpy) {
    float c[3] = {math::cos(0.5f * rpy[0][0]), math::cos(0.5f * rpy[1][0]),
                    math::cos(0.5f * rpy[2][0])};  // cos(*/2)
    float s[3] = {math::sin(0.5f * rpy[0][0]), math::sin(0.5f * rpy[1][0]),
                    math::sin(0.5f * rpy[2][0])};  // sin(*/2)
    auto Q = math::Matrix<float, 4, 1>(
        c[0] * c[1] * c[2] + s[0] * s[1] * s[2],  // q0=cos(θ/2)
        c[0] * c[1] * s[2] - s[0] * s[1] * c[2],  // q1=rx*sin(θ/2)
        c[0] * s[1] * c[2] + s[0] * c[1] * s[2],  // q2=ry*sin(θ/2)
        s[0] * c[1] * c[2] - c[0] * s[1] * s[2]   // q3=rz*sin(θ/2)
    );

    const auto inv_len = math::inv_mag(Q.at<0, 0>(), Q.at<1, 0>(), Q.at<2, 0>(), Q.at<3, 0>());
    return Q * inv_len;
}

/**
 * 将四元数转换为轴角表示
 * @param q 包含[q0, q1, q2, q3]的4x1向量
 * @return 包含[rx, ry, rz, theta]的4x1向量
 */
static constexpr math::Matrix<float, 4, 1> quat2angvec(const math::Matrix<float, 4, 1>& q) {
    float cosq0;
    float theta = 2 * math::acos(math::clamp<float>(q[0][0], -1, 1));
    if (theta == 0 || theta == static_cast<float>(M_PI)) {
        return math::Matrix<float, 4, 1>(0, 0, 0, theta);
    }
    const auto vec = q.submatrix<3, 1>(1, 0);
    const auto inv_vec_len = vec.frobenius_norm();
    return math::Matrix<float, 4, 1>(
        vec[0][0] * inv_vec_len,  // rx
        vec[1][0] * inv_vec_len,  // ry
        vec[2][0] * inv_vec_len,  // rz
        theta                    // theta
    );
}

/**
 * 将轴角表示转换为四元数
 * @param angvec 包含[rx, ry, rz, theta]的4x1向量
 * @return 包含[q0, q1, q2, q3]的4x1向量
 */
static constexpr math::Matrix<float, 4, 1> angvec2quat(const math::Matrix<float, 4, 1>& angvec) {
    float c = math::cos(0.5f * angvec[3][0]), s = math::sin(0.5f * angvec[3][0]);
    const auto Q = math::Matrix<float, 4, 1>{
        c,                 // q0=cos(θ/2)
        s * angvec[0][0],  // q1=rx*sin(θ/2)
        s * angvec[1][0],  // q2=ry*sin(θ/2)
        s * angvec[2][0]   // q3=rz*sin(θ/2)
    };
    return Q / Q.frobenius_norm();
}

/**
 * 从齐次变换矩阵提取旋转部分
 * @param T 4x4齐次变换矩阵
 * @return 3x3旋转矩阵
 */
static constexpr math::Matrix<float, 3, 3> t2r(const math::Matrix<float, 4, 4>& T) {
    return T.submatrix<3, 3>(0, 0);  // R=T(1:3,1:3)
}

/**
 * 将旋转矩阵扩展为齐次变换矩阵（位置为零）
 * @param R 3x3旋转矩阵
 * @return 4x4齐次变换矩阵
 */
static constexpr math::Matrix<float, 4, 4> r2t(const math::Matrix<float, 3, 3>& R) {
  // T=[R,0;0,1]
    return math::Matrix<float, 4, 4>(
        R[0][0], R[0][1], R[0][2], 0, 
        R[1][0], R[1][1], R[1][2], 0,
        R[2][0], R[2][1], R[2][2], 0, 
        0,       0,       0,       1
    );
}

/**
 * 从齐次变换矩阵提取平移部分
 * @param T 4x4齐次变换矩阵
 * @return 3x1平移向量
 */
static constexpr math::Matrix<float, 3, 1> t2p(const math::Matrix<float, 4, 4>& T) {
    return T.submatrix<3, 1>(0, 3);  // p=T(1:3,4)
}

/**
 * 将平移向量扩展为齐次变换矩阵（旋转为单位阵）
 * @param p 3x1平移向量
 * @return 4x4齐次变换矩阵
 */
static constexpr math::Matrix<float, 4, 4> p2t(const math::Matrix<float, 3, 1>& p) {
    // T=[I,P;0,1]
    return math::Matrix<float, 4, 4>(
        1, 0, 0, p[0][0], 
        0, 1, 0, p[1][0],
        0, 0, 1, p[2][0], 
        0, 0, 0, 1
    );
}

/**
 * 组合旋转矩阵和平移向量为齐次变换矩阵
 * @param R 3x3旋转矩阵
 * @param p 3x1平移向量
 * @return 4x4齐次变换矩阵
 */
static constexpr math::Matrix<float, 4, 4> rp2t(const math::Matrix<float, 3, 3>& R, const math::Matrix<float, 3, 1>& p) {
    // T=[R,P;0,1]
    return math::Matrix<float, 4, 4>(
        R[0][0], R[0][1], R[0][2], p[0][0], 
        R[1][0], R[1][1], R[1][2], p[1][0], 
        R[2][0], R[2][1], R[2][2], p[2][0],
        0,       0,       0,       1
    );
}

/**
 * 计算齐次变换矩阵的逆矩阵
 * @param T 4x4齐次变换矩阵
 * @return T的逆矩阵
 */
static constexpr math::Matrix<float, 4, 4> invT(const math::Matrix<float, 4, 4>& T) {
    math::Matrix<float, 3, 3> RT = t2r(T).transpose();
    math::Matrix<float, 3, 1> p_ = RT * t2p(T) * (-1.0);
    return math::Matrix<float, 4, 4>(
        RT[0][0], RT[0][1], RT[0][2], p_[0][0], 
        RT[1][0], RT[1][1], RT[1][2], p_[1][0], 
        RT[2][0], RT[2][1], RT[2][2], p_[2][0],
        0,        0,        0,        1
    );
}

/**
 * 从齐次变换矩阵提取ZYX欧拉角
 * @param T 4x4齐次变换矩阵
 * @return 包含[yaw, pitch, roll]的3x1向量
 */
static constexpr math::Matrix<float, 3, 1> t2rpy(const math::Matrix<float, 4, 4>& T) {
    return r2rpy(t2r(T));
}

/**
 * 将ZYX欧拉角转换为齐次变换矩阵（位置为零）
 * @param rpy 包含[yaw, pitch, roll]的3x1向量
 * @return 4x4齐次变换矩阵
 */
static constexpr math::Matrix<float, 4, 4> rpy2t(const math::Matrix<float, 3, 1>& rpy) {
    return r2t(rpy2r(rpy));
}

/**
 * 从齐次变换矩阵提取轴角表示
 * @param T 4x4齐次变换矩阵
 * @return 包含[rx, ry, rz, theta]的4x1向量
 */
static constexpr math::Matrix<float, 4, 1> t2angvec(const math::Matrix<float, 4, 4>& T) {
    return r2angvec(t2r(T));
}

/**
 * 将轴角表示转换为齐次变换矩阵（位置为零）
 * @param angvec 包含[rx, ry, rz, theta]的4x1向量
 * @return 4x4齐次变换矩阵
 */
static constexpr math::Matrix<float, 4, 4> angvec2t(const math::Matrix<float, 4, 1>& angvec) {
    return r2t(angvec2r(angvec));
}

/**
 * 从齐次变换矩阵提取四元数表示
 * @param T 4x4齐次变换矩阵
 * @return 包含[q0, q1, q2, q3]的4x1向量
 */
static constexpr math::Matrix<float, 4, 1> t2quat(const math::Matrix<float, 4, 4>& T) {
    return r2quat(t2r(T));
}

/**
 * 将四元数转换为齐次变换矩阵（位置为零）
 * @param quat 包含[q0, q1, q2, q3]的4x1向量
 * @return 4x4齐次变换矩阵
 */
static constexpr math::Matrix<float, 4, 4> quat2t(const math::Matrix<float, 4, 1>& quat) {
    return r2t(quat2r(quat));
}

/**
 * 将齐次变换矩阵转换为六维位姿向量（平移+旋转向量）
 * @param T 4x4齐次变换矩阵
 * @return 6x1位姿向量[px, py, pz, rx*theta, ry*theta, rz*theta]
 */
static constexpr math::Matrix<float, 6, 1> t2twist(const math::Matrix<float, 4, 4>& T) {
    math::Matrix<float, 3, 1> p = t2p(T);
    math::Matrix<float, 4, 1> angvec = t2angvec(T);
    math::Matrix<float, 3, 1> phi = angvec.submatrix<3, 1>(0, 0) * angvec[3][0];
    return math::Matrix<float, 6, 1>(
        p[0][0], p[1][0], p[2][0], phi[0][0], phi[1][0], phi[2][0]
    );
}

/**
 * 将六维位姿向量转换为齐次变换矩阵
 * @param twist 6x1位姿向量[px, py, pz, rx*theta, ry*theta, rz*theta]
 * @return 4x4齐次变换矩阵
 */
static constexpr math::Matrix<float, 4, 4> twist2t(const math::Matrix<float, 6, 1>& twist) {
    math::Matrix<float, 3, 1> p = twist.submatrix<3, 1>(0, 0);
    float theta = twist.submatrix<3, 1>(3, 0).frobenius_norm();
    auto angvec = math::Matrix<float, 4, 1>::from_uninitialized();
    angvec.data()[3] = theta;
    if (theta != 0) {
        angvec.data()[0] = twist[3][0] / theta;
        angvec.data()[1] = twist[4][0] / theta;
        angvec.data()[2] = twist[5][0] / theta;
    }else{
        angvec.data()[0] = 0;
        angvec.data()[1] = 0;
        angvec.data()[2] = 0;
    }
    return rp2t(angvec2r(angvec), p);
}

}