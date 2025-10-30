#include "mahony.hpp"
#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"


using namespace ymd;

#if 0

template<arithmetic T>
__fast_inline constexpr
auto build_norm_vec3_from_quat_xfrom_top(
    const T x, const T y, const T z, const T w){

    return Vec3<Norm<T>>(
        Norm(2 * (qconv(Norm(x) , Norm(z) , Norm(- w), Norm(y)))),
        Norm(2 * (qconv(Norm(w) , Norm(x) , Norm(  y), Norm(z)))),
        Norm(qsqu(Norm(w)) - qsqu(Norm(x)) - qsqu(Norm(y)) + qsqu(Norm(z)))
    );
}

template<arithmetic T>
__fast_inline constexpr
Vec3<Norm<T>> build_norm_vec3_from_vec3(const Vec3<T> & vec){
	const auto ilen = isqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	return Vec3<Norm<T>>(
		Norm(vec.x * ilen),
		Norm(vec.y * ilen),
		Norm(vec.z * ilen)
	);
}


template<arithmetic T>
__fast_inline constexpr
Vec3<Norm<T>> build_norm_vec3_from_cross_of_vec3(Vec3<Norm<T>> const & v1, Vec3<Norm<T>> const & v2){
	return Vec3<Norm<T>>(
		Norm(qconv(v1.y, v2.z, - v1.z, v2.y)),
		Norm(qconv(v1.z, v2.x, - v1.x, v2.z)), 
		Norm(qconv(v1.x, v2.y, - v1.y, v2.x))
	);
}

void Mahony::update_v2(const IV3 & gyr,const IV3 & acc) {
	// const auto v = q.xform_top();
	const auto v = build_norm_vec3_from_quat_xfrom_top(q.x, q.y, q.z, q.w);

	// const auto e = acc.normalized().cross(v);

	const auto a_norm = build_norm_vec3_from_vec3(acc);
	const auto e = build_norm_vec3_from_cross_of_vec3(a_norm, v);

	const auto delta = q.xform((gyr + kp_ * e) * (dt_));
	q = (q * IQuat::from_euler_angles<EulerAnglePolicy::XYZ>({delta.x, delta.y, delta.z})).normalized();
}


void Mahony::myupdate(const IV3 & gyr,const IV3 & acc) {

	// const auto x1 = state_[0];
	// const auto x2 = state_[1];

	// state_[0] += x2 * dt; 
	// state_[1] += (- 2 * r * x2 - r_2 * (x1 - u)) * dt;


	// const auto err = IQuat(acc) .cross(q.xform_up());
	// const auto err = acc.cross(v);
	// const auto delta = gyr + (kp_ * err) + ki_ * (inte_ += gyr * dt_);
	// const auto delta = IV3<real_t>(0,0,real_t(2 * TAU));
	const auto pos_err = IQuat::from_direction(acc) * q.inverse();
	// q = q.integral(gyr_hat_, dt_).slerp(v, 1);
	// const auto pos_err_euler = pos_err.to_euler();
	gyr_hat_ = LERP(gyr_hat_, gyr, 0.5_r);
	//  + IV3(pos_err_euler.x, pos_err_euler.y, pos_err_euler.z) * 5000 * dt_;
	q = q.integral(gyr_hat_, dt_).slerp(pos_err, 0.1_r);
	// q =v;
}



void Mahony::myupdate_v2(const IV3 & gyr,const IV3 & acc) {

	// const auto x1 = state_[0];
	// const auto x2 = state_[1];

	// state_[0] += x2 * dt; 
	// state_[1] += (- 2 * r * x2 - r_2 * (x1 - u)) * dt;


	// const auto err = IQuat(acc) .cross(q.xform_up());
	// const auto err = acc.cross(v);
	// const auto delta = gyr + (kp_ * err) + ki_ * (inte_ += gyr * dt_);
	// const auto delta = IV3<real_t>(0,0,real_t(2 * TAU));
	// const auto pos_err =  * q.inverse();
	// q = q.integral(gyr_hat_, dt_).slerp(v, 1);
	// const auto pos_err_euler = pos_err.to_euler();
	static constexpr auto tau = 0.7_r;
	// const auto euler = pos_err.to_euler();
	// const auto gyr_hat_d = tau * tau * IV3{euler.x, euler.y, euler.z} + 2 * tau * (kd_ * (gyr - gyr_hat_));
	const auto gyr_hat_d = 2 * tau * (gyr - gyr_hat_);
	// const auto gyr_hat_d = tau * tau * IV3{euler.x, euler.y, euler.z};
	//  + 2 * tau * (gyr - gyr_hat_);
	gyr_hat_ += gyr_hat_d * dt_;
	//  + IV3(pos_err_euler.x, pos_err_euler.y, pos_err_euler.z) * 5000 * dt_;
	q = q.integral(gyr_hat_, dt_).slerp(IQuat::from_direction(acc), 0.017_r);
	// q =v;
}

#endif

void Mahony::update(const IV3 & gyr,const IV3 & acc) {
	const auto v = q.xform_up();
	const auto err = acc.normalized().cross(v);
	const auto delta = gyr + kp_ * err + ki_ * (inte_ += err * dt_);
	q = q.integral(delta, dt_);
}