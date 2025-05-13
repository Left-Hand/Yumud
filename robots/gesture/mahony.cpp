#include "mahony.hpp"
#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"


using namespace ymd;

template<arithmetic T>
__fast_inline constexpr
auto build_norm_vec3_from_quat_xfrom_top(
    const T x, const T y, const T z, const T w){

    return Vector3_t<Norm_t<T>>(
        Norm_t(2 * (qconv(Norm_t(x) , Norm_t(z) , Norm_t(- w), Norm_t(y)))),
        Norm_t(2 * (qconv(Norm_t(w) , Norm_t(x) , Norm_t(  y), Norm_t(z)))),
        Norm_t(qsqu(Norm_t(w)) - qsqu(Norm_t(x)) - qsqu(Norm_t(y)) + qsqu(Norm_t(z)))
    );
}

template<arithmetic T>
__fast_inline constexpr
Vector3_t<Norm_t<T>> build_norm_vec3_from_vec3(const Vector3_t<T> & vec){
	const auto ilen = isqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	return Vector3_t<Norm_t<T>>(
		Norm_t(vec.x * ilen),
		Norm_t(vec.y * ilen),
		Norm_t(vec.z * ilen)
	);
}


template<arithmetic T>
__fast_inline constexpr
Vector3_t<Norm_t<T>> build_norm_vec3_from_cross_of_vec3(Vector3_t<Norm_t<T>> const & v1, Vector3_t<Norm_t<T>> const & v2){
	return Vector3_t<Norm_t<T>>(
		Norm_t(qconv(v1.y, v2.z, - v1.z, v2.y)),
		Norm_t(qconv(v1.z, v2.x, - v1.x, v2.z)), 
		Norm_t(qconv(v1.x, v2.y, - v1.y, v2.x))
	);
}

void Mahony::update_v2(const Vector3 & gyr,const Vector3 & acc) {
	// const auto v = q.xform_top();
	const auto v = build_norm_vec3_from_quat_xfrom_top(q.x, q.y, q.z, q.w);

	// const auto e = acc.normalized().cross(v);

	const auto a_norm = build_norm_vec3_from_vec3(acc);
	const auto e = build_norm_vec3_from_cross_of_vec3(a_norm, v);

	const auto delta = q.xform((gyr + kp_ * e) * (dt_));
	q = (q * Quat::from_euler<EulerAnglePolicy::XYZ>({delta.x, delta.y, delta.z})).normalized();
}


void Mahony::myupdate(const Vector3 & gyr,const Vector3 & acc) {

	// const auto x1 = state_[0];
	// const auto x2 = state_[1];

	// state_[0] += x2 * dt; 
	// state_[1] += (- 2 * r * x2 - r_2 * (x1 - u)) * dt;


	// const auto err = Quat(acc) .cross(q.xform_up());
	// const auto err = acc.cross(v);
	// const auto delta = gyr + (kp_ * err) + ki_ * (inte_ += gyr * dt_);
	// const auto delta = Vector3_t<real_t>(0,0,real_t(2 * TAU));
	const auto pos_err = Quat::from_direction(acc) * q.inverse();
	// q = q.integral(gyr_hat_, dt_).slerp(v, 1);
	// const auto pos_err_euler = pos_err.to_euler();
	gyr_hat_ = LERP(gyr_hat_, gyr, 0.5_r);
	//  + Vector3_t(pos_err_euler.x, pos_err_euler.y, pos_err_euler.z) * 5000 * dt_;
	q = q.integral(gyr_hat_, dt_).slerp(pos_err, 0.1_r);
	// q =v;
}



void Mahony::myupdate_v2(const Vector3 & gyr,const Vector3 & acc) {

	// const auto x1 = state_[0];
	// const auto x2 = state_[1];

	// state_[0] += x2 * dt; 
	// state_[1] += (- 2 * r * x2 - r_2 * (x1 - u)) * dt;


	// const auto err = Quat(acc) .cross(q.xform_up());
	// const auto err = acc.cross(v);
	// const auto delta = gyr + (kp_ * err) + ki_ * (inte_ += gyr * dt_);
	// const auto delta = Vector3_t<real_t>(0,0,real_t(2 * TAU));
	// const auto pos_err =  * q.inverse();
	// q = q.integral(gyr_hat_, dt_).slerp(v, 1);
	// const auto pos_err_euler = pos_err.to_euler();
	static constexpr auto tau = 0.7_r;
	// const auto euler = pos_err.to_euler();
	// const auto gyr_hat_d = tau * tau * Vector3{euler.x, euler.y, euler.z} + 2 * tau * (kd_ * (gyr - gyr_hat_));
	const auto gyr_hat_d = 2 * tau * (gyr - gyr_hat_);
	// const auto gyr_hat_d = tau * tau * Vector3{euler.x, euler.y, euler.z};
	//  + 2 * tau * (gyr - gyr_hat_);
	gyr_hat_ += gyr_hat_d * dt_;
	//  + Vector3_t(pos_err_euler.x, pos_err_euler.y, pos_err_euler.z) * 5000 * dt_;
	q = q.integral(gyr_hat_, dt_).slerp(Quat::from_direction(acc), 0.017_r);
	// q =v;
}

void Mahony::update(const Vector3 & gyr,const Vector3 & acc) {
	const auto v = q.xform_up();
	const auto err = acc.normalized().cross(v);
	const auto delta = gyr + kp_ * err + ki_ * (inte_ += err * dt_);
	q = q.integral(delta, dt_);
}