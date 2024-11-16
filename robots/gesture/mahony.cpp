#include "mahony.hpp"

using namespace ymd;

Mahony::Quat Mahony::update(const Vector3 & _gyr,const Vector3 & _acc){

	Vector3 gyr = _gyr;
	Vector3 acc = _acc;
	
	real_t halfvx, halfvy, halfvz;	// 估计的重力加速度矢量，half表示半值
	real_t halfex, halfey, halfez; // 误差向量
	real_t & q0 = q[0],q1 = q[1], q2 = q[2], q3 = q[3];
	
	real_t q0q0 = q[0]*q[0];
	real_t q0q1 = q[0]*q[1];
	real_t q0q2 = q[0]*q[2];
	real_t q1q3 = q[1]*q[3];
	real_t q2q3 = q[2]*q[3];
	real_t q3q3 = q[3]*q[3];  

	// 只在加速度计数据有效时才进行运算
	if((acc)) {
		acc.normalize();

		// 通过四元数（旋转矩阵）得到理论重力加速度在机体坐标系下的向量值 
		// 注意，这里实际上是矩阵第三列*1/2，在开头对Kp Ki的宏定义均为2*增益，这样处理目的是减少乘法运算量
		halfvx = q1q3 - q0q2;
		halfvy = q0q1 + q2q3;
		halfvz = q0q0 - real_t(0.5) + q3q3;	// q0q0 - q1q1 - q2q2 + q3q3 的优化版（|q| =1）
	
		// 求误差：实际重力加速度向量（测量值）与理论重力加速度向量（估计值）做外积
		halfex = (acc.y * halfvz - acc.z * halfvy);
		halfey = (acc.z * halfvx - acc.x * halfvz);
		halfez = (acc.x * halfvy - acc.y * halfvx);

		if(twoKi > 0){
			// 积分过程
			integralFBx += twoKi * halfex * invSampleFreq;	
			integralFBy += twoKi * halfey * invSampleFreq;
			integralFBz += twoKi * halfez * invSampleFreq;

			// 积分项
			gyr.x += integralFBx;
			gyr.y += integralFBy;
			gyr.z += integralFBz;
		}
		else {
			integralFBx = 0;	
			integralFBy = 0;
			integralFBz = 0;
		}

		// 比例项
		gyr.x += twoKp * halfex;
		gyr.y += twoKp * halfey;
		gyr.z += twoKp * halfez;
	}
	
	// 四元数 微分方程
	gyr.x *= (invSampleFreq >> 1);
	gyr.y *= (invSampleFreq >> 1);
	gyr.z *= (invSampleFreq >> 1);
	
	q[0] += (-q1 * gyr.x - q2 * gyr.y - q3 * gyr.z);
	q[1] += (q0 * gyr.x + q2 * gyr.z - q3 * gyr.y);
	q[2] += (q0 * gyr.y - q1 * gyr.z + q3 * gyr.x);
	q[3] += (q0 * gyr.z + q1 * gyr.y - q2 * gyr.x); 
	

    q.normalize();
	return q;
}


Mahony::Quat Mahony::update9(const Vector3 & _gyr,const Vector3 & _acc,const Vector3 & _mag){

	Vector3 gyr = _gyr;
	Vector3 acc = _acc;
	Vector3 mag = _mag;

	real_t recipNorm;
	real_t & q0 = q[0],q1 = q[1], q2 = q[2], q3 = q[3];

	real_t hx, hy, bx, bz;
	real_t halfvx, halfvy, halfvz, halfwx, halfwy, halfwz;
	real_t halfex, halfey, halfez;
	real_t qa, qb, qc;

	auto [gx,gy,gz] = gyr;
	// Compute feedback only if accerometer measurement valid
	// (avoids NaN in accerometer normalisation)
	if(acc) {

		acc.normalize();
		mag.normalize();

		auto [ax,ay,az] = acc;
		auto [mx,my,mz] = mag;

		// Auxiliary variables to avoid repeated arithmetic
		real_t q0q0 = q0 * q0;
		real_t q0q1 = q0 * q1;
		real_t q0q2 = q0 * q2;
		real_t q0q3 = q0 * q3;
		real_t q1q1 = q1 * q1;
		real_t q1q2 = q1 * q2;
		real_t q1q3 = q1 * q3;
		real_t q2q2 = q2 * q2;
		real_t q2q3 = q2 * q3;
		real_t q3q3 = q3 * q3;

		// Reference direction of Earth's magnetic field
		hx = 2 * (mx * (real_t(0.5) - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
		hy = 2 * (mx * (q1q2 + q0q3) + my * (real_t(0.5) - q1q1 - q3q3) + mz * (q2q3 - q0q1));
		bx = sqrt(hx * hx + hy * hy);
		bz = 2 * (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) + mz * (real_t(0.5) - q1q1 - q2q2));

		// Estimated direction of gravity and magnetic field
		halfvx = q1q3 - q0q2;
		halfvy = q0q1 + q2q3;
		halfvz = q0q0 - real_t(0.5) + q3q3;
		halfwx = bx * (real_t(0.5) - q2q2 - q3q3) + bz * (q1q3 - q0q2);
		halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
		halfwz = bx * (q0q2 + q1q3) + bz * (real_t(0.5) - q1q1 - q2q2);

		// Error is sum of cross product between estimated direction
		// and measured direction of field vectors
		halfex = (ay * halfvz - az * halfvy) + (my * halfwz - mz * halfwy);
		halfey = (az * halfvx - ax * halfvz) + (mz * halfwx - mx * halfwz);
		halfez = (ax * halfvy - ay * halfvx) + (mx * halfwy - my * halfwx);

		// Compute and apply integral feedback if enabled
		if(twoKi > 0) {
			// integral error scaled by Ki
			integralFBx += twoKi * halfex * invSampleFreq;
			integralFBy += twoKi * halfey * invSampleFreq;
			integralFBz += twoKi * halfez * invSampleFreq;
			gx += integralFBx;	// apply integral feedback
			gy += integralFBy;
			gz += integralFBz;
		} else {
			integralFBx = 0;	// prevent integral windup
			integralFBy = 0;
			integralFBz = 0;
		}

		// Apply proportional feedback
		gx += twoKp * halfex;
		gy += twoKp * halfey;
		gz += twoKp * halfez;
	}

	// Integrate rate of change of quaternion
	gx *= (real_t(0.5) * invSampleFreq);		// pre-multiply common factors
	gy *= (real_t(0.5) * invSampleFreq);
	gz *= (real_t(0.5) * invSampleFreq);
	
	qa = q0;
	qb = q1;
	qc = q2;
	
	q0 += (-qb * gx - qc * gy - q3 * gz);
	q1 += (qa * gx + qc * gz - q3 * gy);
	q2 += (qa * gy - qb * gz + q3 * gx);
	q3 += (qa * gz + qb * gy - qc * gx);

	q.normalize();

	return q;
}