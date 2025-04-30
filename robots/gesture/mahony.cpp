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

	const auto delta = q.xform((gyr + kp_ * e) * (inv_fs_));
	q = (q * Quat::from_euler<EulerAnglePolicy::XYZ>({delta.x, delta.y, delta.z})).normalized();
}


void Mahony::update(const Vector3 & gyr,const Vector3 & acc) {
	const auto v = q.xform_top();
	const auto err = acc.normalized().cross(v);
	const auto delta = gyr + kp_ * err + ki_ * (inte_ += err * inv_fs_);
	q = q.integral((delta) * (inv_fs_));
}