#pragma once

/**
 * @file imuFusion.h
 * @author ZheWana(zhewana.cn)
 * @brief imuFusion header file
 * @date 2022/4/20
 * @note Do not modified the code if you do not know what you are doing!!!
 * @refer MahonyAHRS
 *                  ___           ___           ___           ___           ___                       ___           ___
 *      ___        /\__\         /\__\         /\  \         /\__\         /\  \          ___        /\  \         /\__\
 *     /\  \      /::|  |       /:/  /        /::\  \       /:/  /        /::\  \        /\  \      /::\  \       /::|  |
 *     \:\  \    /:|:|  |      /:/  /        /:/\:\  \     /:/  /        /:/\ \  \       \:\  \    /:/\:\  \     /:|:|  |
 *     /::\__\  /:/|:|__|__   /:/  /  ___   /::\~\:\  \   /:/  /  ___   _\:\~\ \  \      /::\__\  /:/  \:\  \   /:/|:|  |__
 *  __/:/\/__/ /:/ |::::\__\ /:/__/  /\__\ /:/\:\ \:\__\ /:/__/  /\__\ /\ \:\ \ \__\  __/:/\/__/ /:/__/ \:\__\ /:/ |:| /\__\
 * /\/:/  /    \/__/~~/:/  / \:\  \ /:/  / \/__\:\ \/__/ \:\  \ /:/  / \:\ \:\ \/__/ /\/:/  /    \:\  \ /:/  / \/__|:|/:/  /
 * \::/__/           /:/  /   \:\  /:/  /       \:\__\    \:\  /:/  /   \:\ \:\__\   \::/__/      \:\  /:/  /      |:/:/  /
 *  \:\__\          /:/  /     \:\/:/  /         \/__/     \:\/:/  /     \:\/:/  /    \:\__\       \:\/:/  /       |::/  /
 *   \/__/         /:/  /       \::/  /                     \::/  /       \::/  /      \/__/        \::/  /        /:/  /
 *                 \/__/         \/__/                       \/__/         \/__/                     \/__/         \/__/
 */

#include "sys/core/platform.h"
#include "types/vector3/vector3_t.hpp"
#include "types/basis/Basis_t.hpp"
#include "types/quat/Quat_t.hpp"

namespace ymd{

class ImuFusion{
public:
    using Vector3 = ymd::Vector3_t<real_t>;
    using Basis = ymd::Basis_t<real_t>;
    using Quat = ymd::Quat_t<real_t>;
protected:
    real_t vx, vy, vz;
    real_t aex, aey, aez;
    real_t mex, mey, mez;
    real_t aexInt, aeyInt, aezInt;
    real_t mexInt, meyInt, mezInt;

    real_t delta; // 解算周期

    Quat quat_;
public:
    void update9(const Vector3 & gyr, const Vector3 & acc, const Vector3 & mag);

    auto quat() const { return quat_;}
    auto euler() const {return Basis(quat_).get_euler_xyz();} 
};

}