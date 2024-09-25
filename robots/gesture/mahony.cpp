#include "mahony.hpp"

Quat Mahony::update(Vector3 gyro,Vector3 accel){
	Quat q;
	
	real_t halfvx, halfvy, halfvz;	// 估计的重力加速度矢量，half表示半值
	real_t halfex, halfey, halfez; // 误差向量
	real_t q0 = q[0],q1 = q[1], q2 = q[2], q3 = q[3];
	
	real_t q0q0 = q[0]*q[0];
	real_t q0q1 = q[0]*q[1];
	real_t q0q2 = q[0]*q[2];
	real_t q1q3 = q[1]*q[3];
	real_t q2q3 = q[2]*q[3];
	real_t q3q3 = q[3]*q[3];  

	// 只在加速度计数据有效时才进行运算
	if((accel)) {
		accel.normalize();

		// 通过四元数（旋转矩阵）得到理论重力加速度在机体坐标系下的向量值 
		// 注意，这里实际上是矩阵第三列*1/2，在开头对Kp Ki的宏定义均为2*增益，这样处理目的是减少乘法运算量
		halfvx = q1q3 - q0q2;
		halfvy = q0q1 + q2q3;
		halfvz = q0q0 - real_t(0.5f) + q3q3;	// q0q0 - q1q1 - q2q2 + q3q3 的优化版（|q| =1）
	
		// 求误差：实际重力加速度向量（测量值）与理论重力加速度向量（估计值）做外积
		halfex = (accel.y * halfvz - accel.z * halfvy);
		halfey = (accel.z * halfvx - accel.x * halfvz);
		halfez = (accel.x * halfvy - accel.y * halfvx);

		if(twoKi > 0){
			// 积分过程
			integralFBx += twoKi * halfex * (1 / sampleFreq);	
			integralFBy += twoKi * halfey * (1 / sampleFreq);
			integralFBz += twoKi * halfez * (1 / sampleFreq);

			// 积分项
			gyro.x += integralFBx;
			gyro.y += integralFBy;
			gyro.z += integralFBz;
		}
		else {
			integralFBx = 0;	
			integralFBy = 0;
			integralFBz = 0;
		}

		// 比例项
		gyro.x += twoKp * halfex;
		gyro.y += twoKp * halfey;
		gyro.z += twoKp * halfez;
	}
	
	// 四元数 微分方程
	gyro.x *= ((1 / sampleFreq) >> 1);
	gyro.y *= ((1 / sampleFreq) >> 1);
	gyro.z *= ((1 / sampleFreq) >> 1);
	
	q[0] += (-q1 * gyro.x - q2 * gyro.y - q3 * gyro.z);
	q[1] += (q0 * gyro.x + q2 * gyro.z - q3 * gyro.y);
	q[2] += (q0 * gyro.y - q1 * gyro.z + q3 * gyro.x);
	q[3] += (q0 * gyro.z + q1 * gyro.y - q2 * gyro.x); 
	

    q.normalize();
	return q;
}