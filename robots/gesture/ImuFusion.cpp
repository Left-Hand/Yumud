#include "ImuFusion.hpp"

using namespace ymd;


// 设置加速度修正PI值
#define Accel_Kp (3)
#define Accel_Ki (0)
// 设置磁力计修正PI值
#define Magnetic_Kp (5) // 10.0f
#define Magnetic_Ki (0)


#define Fusion_Initial_X_North

void ImuFusion::update9(const Vector3 & gyro, const Vector3 & accel, const Vector3 & mag){

    real_t hx, hy; // 旋转到参考坐标系的机体磁场
    real_t bz; // 参考坐标系的地磁场
    real_t wx, wy, wz; // 误差向量

    scexpr real_t akp = Accel_Kp;
    scexpr real_t aki = Accel_Ki;
    scexpr real_t mkp = Magnetic_Kp;
    scexpr real_t mki = Magnetic_Ki; // 170 0.05f
    
    real_t q0q0 = this->quat.x * this->quat.x;
    real_t q0q1 = this->quat.x * this->quat.y;
    real_t q0q2 = this->quat.x * this->quat.z;
    real_t q0q3 = this->quat.x * this->quat.w;
    real_t q1q1 = this->quat.y * this->quat.y;
    real_t q1q2 = this->quat.y * this->quat.z;
    real_t q1q3 = this->quat.y * this->quat.w;
    real_t q2q2 = this->quat.z * this->quat.z;
    real_t q2q3 = this->quat.z * this->quat.w;
    real_t q3q3 = this->quat.w * this->quat.w;

    auto [Ax, Ay, Az] = accel.normalized();
    auto [Gx, Gy, Gz] = gyro; // 角速度信息 - 用户输入
    auto [Mx, My, Mz] = mag.normalized(); // 磁力计信息 - 用户输入
    
    //参考坐标系中的重力向量旋转到载体坐标系
    vx = 2 * (q1q3 - q0q2);
    vy = 2 * (q0q1 + q2q3);
    vz = 2 * (q0q0 + q3q3) - 1;

    //估算向量与实际向量求外积，获得向量间的误差（此处所指误差正比于轴角
    aex = (Ay * vz - Az * vy);
    aey = (Az * vx - Ax * vz);
    aez = (Ax * vy - Ay * vx);

    //对误差做PI控制，用以调整陀螺仪数据
    aexInt += aex;
    aeyInt += aey;
    aezInt += aez;

    //磁力计矫正
    if (Mx != 0 && My != 0 && Mz != 0) {

        hx = 2 * (Mx * (real_t(0.5f) - q2q2 - q3q3) + My * (q1q2 - q0q3) + Mz * (q1q3 + q0q2));
        hy = 2 * (Mx * (q1q2 + q0q3) + My * (real_t(0.5f) - q1q1 - q3q3) + Mz * (q2q3 - q0q1));

#ifdef Fusion_Initial_X_North
        real_t bx = sqrt(hx * hx + hy * hy);
#endif

#ifdef Fusion_Initial_Y_North
        real_t by = sqrt(hx * hx + hy * hy);
#endif
        bz = 2 * (Mx * (q1q3 - q0q2) + // bz = hz
                     My * (q2q3 + q0q1) + Mz * (real_t(0.5f) - q1q1 - q2q2));

        wx = 2 * (bz * (q1q3 - q0q2) +
                     #ifdef Fusion_Initial_X_North
                     bx * (real_t(0.5f) - q2q2 - q3q3));
                     #else
                     by * (q1q2 + q0q3));
#endif
        wy = 2 * (bz * (q0q1 + q2q3) +
                     #ifdef Fusion_Initial_X_North
                     bx * (q1q2 - q0q3));
                     #else
                     by * (real_t(0.5f) - q1q1 - q3q3));
#endif // Fusion_Initial_X_North
        wz = 2 * (bz * (real_t(0.5f) - q1q1 - q2q2) +
                     #ifdef Fusion_Initial_X_North
                     bx * (q0q2 + q1q3));
                     #else
                     by * (q2q3 - q0q1));
#endif // Fusion_Initial_X_North

        mex = (My * wz - Mz * wy);
        mey = (Mz * wx - Mx * wz);
        mez = (Mx * wy - My * wx);

        //对误差做PI控制，用以调整陀螺仪数据
        mexInt += mex;
        meyInt += mey;
        mezInt += mez;
    }

    // 校准陀螺仪数据
    Gx += (aex * akp + aexInt * aki) + (mex * mkp + mexInt * mki);
    Gy += (aey * akp + aeyInt * aki) + (mey * mkp + meyInt * mki);
    Gz += (aez * akp + aezInt * aki) + (mez * mkp + mezInt * mki);

    this->quat.x += real_t(0.5f) * (-this->quat.y * Gx - this->quat.z * Gy - this->quat.w * Gz) * this->delta;
    this->quat.y += real_t(0.5f) * (this->quat.x * Gx + this->quat.z * Gz - this->quat.w * Gy) * this->delta;
    this->quat.z += real_t(0.5f) * (this->quat.x * Gy - this->quat.y * Gz + this->quat.w * Gx) * this->delta;
    this->quat.w += real_t(0.5f) * (this->quat.x * Gz + this->quat.y * Gy - this->quat.z * Gx) * this->delta;

    this->quat.normalize();
}