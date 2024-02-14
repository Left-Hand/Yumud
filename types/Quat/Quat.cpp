// #include "Quat.hpp"

// real_t Quat::angle_to(const Quat &p_to) const {
// 	real_t d = dot(p_to);
// 	return acosf(CLAMP(d * d * 2.0f - 1.0f, -1.0f, 1.0f));
// }
// real_t Quat::dot(const Quat &p_q) const {
// 	return x * p_q.x + y * p_q.y + z * p_q.z + w * p_q.w;
// }
// real_t Quat::length_squared() const{
//     return dot(*this);
// }
// // set_euler_xyz expects a vector containing the Euler angles in the format
// // (ax,ay,az), where ax is the angle of rotation around x axis,
// // and similar for other axes.
// // This implementation uses XYZ convention (Z is the first rotation).
// void Quat::set_euler_xyz(const Vector3 &p_euler) {
// 	real_t half_a1 = p_euler.x * 0.5;
// 	real_t half_a2 = p_euler.y * 0.5;
// 	real_t half_a3 = p_euler.z * 0.5;

// 	// R = X(a1).Y(a2).Z(a3) convention for Euler angles.
// 	// Conversion to quaternion as listed in https://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19770024290.pdf (page A-2)
// 	// a3 is the angle of the first rotation, following the notation in this reference.

// 	real_t cos_a1 = cosf(half_a1);
// 	real_t sin_a1 = sinf(half_a1);
// 	real_t cos_a2 = cosf(half_a2);
// 	real_t sin_a2 = sinf(half_a2);
// 	real_t cos_a3 = cosf(half_a3);
// 	real_t sin_a3 = sinf(half_a3);

// 	set(sin_a1 * cos_a2 * cos_a3 + sin_a2 * sin_a3 * cos_a1,
// 			-sin_a1 * sin_a3 * cos_a2 + sin_a2 * cos_a1 * cos_a3,
// 			sin_a1 * sin_a2 * cos_a3 + sin_a3 * cos_a1 * cos_a2,
// 			-sin_a1 * sin_a2 * sin_a3 + cos_a1 * cos_a2 * cos_a3);
// }

// // get_euler_xyz returns a vector containing the Euler angles in the format
// // (ax,ay,az), where ax is the angle of rotation around x axis,
// // and similar for other axes.
// // This implementation uses XYZ convention (Z is the first rotation).
// // Vector3 Quat::get_euler_xyz() const {
// // 	Basis m(*this);
// // 	return m.get_euler_xyz();
// // }

// // set_euler_yxz expects a vector containing the Euler angles in the format
// // (ax,ay,az), where ax is the angle of rotation around x axis,
// // and similar for other axes.
// // This implementation uses YXZ convention (Z is the first rotation).
// void Quat::set_euler_yxz(const Vector3 &p_euler) {
// 	real_t half_a1 = p_euler.y * 0.5;
// 	real_t half_a2 = p_euler.x * 0.5;
// 	real_t half_a3 = p_euler.z * 0.5;

// 	// R = Y(a1).X(a2).Z(a3) convention for Euler angles.
// 	// Conversion to quaternion as listed in https://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19770024290.pdf (page A-6)
// 	// a3 is the angle of the first rotation, following the notation in this reference.

// 	real_t cos_a1 = cosf(half_a1);
// 	real_t sin_a1 = sinf(half_a1);
// 	real_t cos_a2 = cosf(half_a2);
// 	real_t sin_a2 = sinf(half_a2);
// 	real_t cos_a3 = cosf(half_a3);
// 	real_t sin_a3 = sinf(half_a3);

// 	set(sin_a1 * cos_a2 * sin_a3 + cos_a1 * sin_a2 * cos_a3,
// 			sin_a1 * cos_a2 * cos_a3 - cos_a1 * sin_a2 * sin_a3,
// 			-sin_a1 * sin_a2 * cos_a3 + cos_a1 * cos_a2 * sin_a3,
// 			sin_a1 * sin_a2 * sin_a3 + cos_a1 * cos_a2 * cos_a3);
// }

// // get_euler_yxz returns a vector containing the Euler angles in the format
// // (ax,ay,az), where ax is the angle of rotation around x axis,
// // and similar for other axes.
// // This implementation uses YXZ convention (Z is the first rotation).
// // Vector3 Quat::get_euler_yxz() const {
// // 	Basis m(*this);
// // 	return m.get_euler_yxz();
// // }

// void Quat::operator*=(const Quat &p_q) {
// 	set(w * p_q.x + x * p_q.w + y * p_q.z - z * p_q.y,
// 			w * p_q.y + y * p_q.w + z * p_q.x - x * p_q.z,
// 			w * p_q.z + z * p_q.w + x * p_q.y - y * p_q.x,
// 			w * p_q.w - x * p_q.x - y * p_q.y - z * p_q.z);
// }

// Quat Quat::operator*(const Quat &p_q) const {
// 	Quat r = *this;
// 	r *= p_q;
// 	return r;
// }

// bool Quat::is_equal_approx(const Quat &p_quat) const {
// 	return is_equal_approx(x, p_quat.x) && is_equal_approx(y, p_quat.y) && is_equal_approx(z, p_quat.z) && is_equal_approx(w, p_quat.w);
// }

// real_t Quat::length() const {
// 	return sqrt(length_squared());
// }

// void Quat::normalize() {
// 	*this /= length();
// }

// Quat Quat::normalized() const {
// 	return *this / length();
// }

// bool Quat::is_normalized() const {
// 	return is_equal_approx(length_squared(), real_t(1)); //use less epsilon
// }

// Quat Quat::operator/(const real_t &s) const{
//     return Quat(x/s, y/s, z/s, w/s);
// }

// Quat Quat::inverse() const {
// #ifdef MATH_CHECKS
// 	ERR_FAIL_COND_V_MSG(!is_normalized(), Quat(), "The quaternion must be normalized.");
// #endif
// 	return Quat(-x, -y, -z, w);
// }

// Quat Quat::slerp(const Quat &p_to, const real_t &p_weight) const {
// 	Quat to1;
// 	real_t omega, cosom, sinom, scale0, scale1;

// 	// calc cosine
// 	cosom = dot(p_to);

// 	// adjust signs (if necessary)
// 	if (cosom < 0.0) {
// 		cosom = -cosom;
// 		to1.x = -p_to.x;
// 		to1.y = -p_to.y;
// 		to1.z = -p_to.z;
// 		to1.w = -p_to.w;
// 	} else {
// 		to1.x = p_to.x;
// 		to1.y = p_to.y;
// 		to1.z = p_to.z;
// 		to1.w = p_to.w;
// 	}

// 	// calculate coefficients

// 	if ((1.0 - cosom) > CMP_EPSILON) {
// 		// standard case (slerp)
// 		omega = acos(cosom);
// 		sinom = sinf(omega);
// 		scale0 = sinf((1.0 - p_weight) * omega) / sinom;
// 		scale1 = sinf(p_weight * omega) / sinom;
// 	} else {
// 		// "from" and "to" quaternions are very close
// 		//  ... so we can do a linear interpolation
// 		scale0 = 1.0 - p_weight;
// 		scale1 = p_weight;
// 	}
// 	// calculate final values
// 	return Quat(
// 			scale0 * x + scale1 * to1.x,
// 			scale0 * y + scale1 * to1.y,
// 			scale0 * z + scale1 * to1.z,
// 			scale0 * w + scale1 * to1.w);
// }

// Quat Quat::slerpni(const Quat &p_to, const real_t &p_weight) const {
// 	const Quat &from = *this;

// 	real_t dot = from.dot(p_to);

// 	if (abs(dot) > real_t(0.9999)) {
// 		return from;
// 	}

// 	real_t theta = acos(dot),
//             sinT = 1.0 / sinf(theta),
// 		    newFactor = sinf(p_weight * theta) * sinT,
// 		    invFactor = sinf((1.0 - p_weight) * theta) * sinT;

// 	return Quat(invFactor * from.x + newFactor * p_to.x,
// 			invFactor * from.y + newFactor * p_to.y,
// 			invFactor * from.z + newFactor * p_to.z,
// 			invFactor * from.w + newFactor * p_to.w);
// }

// Quat Quat::cubic_slerp(const Quat &p_b, const Quat &p_pre_a, const Quat &p_post_b, const real_t &p_weight) const {

// 	real_t t2 = (1.0 - p_weight) * p_weight * 2;
// 	Quat sp = this->slerp(p_b, p_weight);
// 	Quat sq = p_pre_a.slerpni(p_post_b, p_weight);
// 	return sp.slerpni(sq, t2);
// }

// void Quat::set_axis_angle(const Vector3 &axis, const real_t &angle) {

// 	real_t d = axis.length();
// 	if (d == 0) {
// 		set(real_t(0), real_t(0), real_t(0), real_t(0));
// 	} else {
// 		real_t sin_angle = sinf(angle * 0.5);
// 		real_t cos_angle = cosf(angle * 0.5);
// 		real_t s = sin_angle / d;
// 		set(axis.x * s, axis.y * s, axis.z * s,
// 				cos_angle);
// 	}
// }
