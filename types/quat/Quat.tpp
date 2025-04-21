/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/* Copyright (c) 2024  Rstr1aN / Yumud                                    */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/*                                                                        */
/* Note: This file has been modified by Rstr1aN / Yumud.                  */
/**************************************************************************/



#define set(p_x, p_y, p_z, p_w)\
	x = T(p_x);\
	y = T(p_y);\
	z = T(p_z);\
	w = T(p_w);\

namespace ymd{
template<typename T>
constexpr T Quat_t<T>::angle_to(const Quat_t<T> &p_to) const {
	T d = dot(p_to);
	return acosf(CLAMP(d * d * 2.0f - 1.0f, -1.0f, 1.0f));
}

template<typename T>
constexpr T Quat_t<T>::dot(const Quat_t<T> &p_q) const {
	return T(x * p_q.x + y * p_q.y + z * p_q.z + w * p_q.w);
}

template<typename T>
constexpr T Quat_t<T>::length_squared() const{
    return dot(*this);
}

template<typename T>
constexpr T Quat_t<T>::inv_length() const {
	return T(isqrt(x * x + y * y + z * z + w * w));
}

// set_euler_xyz expects a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// This implementation uses XYZ convention (Z is the first rotation).
template<typename T>
constexpr void Quat_t<T>::set_euler_xyz(const Vector3_t<T> &p_euler) {
	T half_a1 = p_euler.x / 2;
	T half_a2 = p_euler.y / 2;
	T half_a3 = p_euler.z / 2;

	// R = X(a1).Y(a2).Z(a3) convention for Euler angles.
	// Conversion to Quat_t<T> as listed in https://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19770024290.pdf (page A-2)
	// a3 is the angle of the first rotation, following the notation in this reference.

	auto [sin_a1, cos_a1] = sincos(half_a1);
	auto [sin_a2, cos_a2] = sincos(half_a2);
	auto [sin_a3, cos_a3] = sincos(half_a3);

	set(sin_a1 * cos_a2 * cos_a3 + sin_a2 * sin_a3 * cos_a1,
			-sin_a1 * sin_a3 * cos_a2 + sin_a2 * cos_a1 * cos_a3,
			sin_a1 * sin_a2 * cos_a3 + sin_a3 * cos_a1 * cos_a2,
			-sin_a1 * sin_a2 * sin_a3 + cos_a1 * cos_a2 * cos_a3);
}

// get_euler_xyz returns a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// This implementation uses XYZ convention (Z is the first rotation).
// Vector3_t<T> Quat_t<T>::get_euler_xyz() const {
// 	Basis m(*this);
// 	return m.get_euler_xyz();
// }

// set_euler_yxz expects a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// This implementation uses YXZ convention (Z is the first rotation).
template<typename T>
constexpr void Quat_t<T>::set_euler_yxz(const Vector3_t<T> &p_euler) {
	
	// R = Y(a1).X(a2).Z(a3) convention for Euler angles.
	// Conversion to Quat_t<T> as listed in https://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19770024290.pdf (page A-6)
	// a3 is the angle of the first rotation, following the notation in this reference.

	// if constexpr(is_fixed_point_v<T>){	
	// 	auto [sin_a1, cos_a1] = sincos<14>(p_euler.y >> 1);
	// 	auto [sin_a2, cos_a2] = sincos<14>(p_euler.x >> 1);
	// 	auto [sin_a3, cos_a3] = sincos<14>(p_euler.z >> 1);

	// 	static auto mul3 = [](q14 a, q14 b, q14 c) -> q14{
	// 		return q14(_iq<14>::from_i32(((a.value.to_i32() * b.value.to_i32()) >> 14) * c.value.to_i32() >> 14));
	// 	};
	
	// 	set(
	// 		mul3( sin_a1, cos_a2, sin_a3) + mul3(cos_a1, sin_a2, cos_a3),
	// 		mul3( sin_a1, cos_a2, cos_a3) - mul3(cos_a1, sin_a2, sin_a3),
	// 		mul3(-sin_a1, sin_a2, cos_a3) + mul3(cos_a1, cos_a2, sin_a3),
	// 		mul3( sin_a1, sin_a2, sin_a3) + mul3(cos_a1, cos_a2, cos_a3)
	// 	);
	// }else{
	auto [sin_a1, cos_a1] = sincos(p_euler.y / 2);
	auto [sin_a2, cos_a2] = sincos(p_euler.x / 2);
	auto [sin_a3, cos_a3] = sincos(p_euler.z / 2);

	set(
		sin_a1  * cos_a2 * sin_a3 + cos_a1 * sin_a2 * cos_a3,
		sin_a1  * cos_a2 * cos_a3 - cos_a1 * sin_a2 * sin_a3,
		-sin_a1 * sin_a2 * cos_a3 + cos_a1 * cos_a2 * sin_a3,
		sin_a1  * sin_a2 * sin_a3 + cos_a1 * cos_a2 * cos_a3
	);
	// }

}

// get_euler_yxz returns a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// This implementation uses YXZ convention (Z is the first rotation).
// Vector3_t<T> Quat_t<T>::get_euler_yxz() const {
// 	Basis m(*this);
// 	return m.get_euler_yxz();
// }
template<typename T>
constexpr void Quat_t<T>::operator*=(const Quat_t<T> &p_q) {
	set(    T(w * p_q.x + x * p_q.w + y * p_q.z - z * p_q.y),
			T(w * p_q.y + y * p_q.w + z * p_q.x - x * p_q.z),
			T(w * p_q.z + z * p_q.w + x * p_q.y - y * p_q.x),
			T(w * p_q.w - x * p_q.x - y * p_q.y - z * p_q.z)
		);

	// x = T(w * p_q.x + x * p_q.w + y * p_q.z - z * p_q.y);
	// y = T(w * p_q.y + y * p_q.w + z * p_q.x - x * p_q.z);
	// z = T(w * p_q.z + z * p_q.w + x * p_q.y - y * p_q.x);
	// w = T(w * p_q.w - x * p_q.x - y * p_q.y - z * p_q.z);
}
template<typename T>
constexpr Quat_t<T> Quat_t<T>::operator*(const Quat_t<T> & p_q) const {
	Quat_t<T> r = *this;
	r *= p_q;
	return r;
}

template<typename T>
constexpr Quat_t<T> Quat_t<T>::operator*(Quat_t<T> && p_q) const {
	p_q *= *this;
	return p_q;
}
template<typename T>
constexpr bool Quat_t<T>::is_equal_approx(const Quat_t<T> & other) const {
	return is_equal_approx(x, other.x) && is_equal_approx(y, other.y) && is_equal_approx(z, other.z) && is_equal_approx(w, other.w);
}
template<typename T>
constexpr T Quat_t<T>::length() const {
	return sqrt(length_squared());
}
template<typename T>
constexpr void Quat_t<T>::normalize() {
	*this *= inv_length();
}
template<typename T>
constexpr Quat_t<T> Quat_t<T>::normalized() const {
	return *this * inv_length();
}
template<typename T>
constexpr bool Quat_t<T>::is_normalized() const {
	return is_equal_approx(length_squared(), T(1)); //use less epsilon
}
template<typename T>
constexpr Quat_t<T> Quat_t<T>::operator/(const T &s) const{
	const T inv_s = 1 / s;
    return Quat_t<T>(x * inv_s, y * inv_s, z * inv_s, w * inv_s);
}
template<typename T>
constexpr Quat_t<T> Quat_t<T>::inverse() const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!is_normalized(), Quat_t<T>(), "The Quat_t<T> must be normalized.");
#endif
	return Quat_t<T>(-x, -y, -z, w);
}
template<typename T>
constexpr Quat_t<T> Quat_t<T>::slerp(const Quat_t<T> &p_to, const T &p_weight) const {
	Quat_t<T> to1;
	T omega, cosom, sinom, scale0, scale1;

	// calc cosine
	cosom = dot(p_to);

	// adjust signs (if necessary)
	if (cosom < 0.0) {
		cosom = -cosom;
		to1.x = -p_to.x;
		to1.y = -p_to.y;
		to1.z = -p_to.z;
		to1.w = -p_to.w;
	} else {
		to1.x = p_to.x;
		to1.y = p_to.y;
		to1.z = p_to.z;
		to1.w = p_to.w;
	}

	// calculate coefficients

	if ((1.0 - cosom) > CMP_EPSILON) {
		// standard case (slerp)
		omega = acos(cosom);
		sinom = sinf(omega);
		scale0 = sinf((1 - p_weight) * omega) / sinom;
		scale1 = sinf(p_weight * omega) / sinom;
	} else {
		// "from" and "to" Quat_t<T>s are very close
		//  ... so we can do a linear interpolation
		scale0 = 1.0 - p_weight;
		scale1 = p_weight;
	}
	// calculate final values
	return Quat_t<T>(
			scale0 * x + scale1 * to1.x,
			scale0 * y + scale1 * to1.y,
			scale0 * z + scale1 * to1.z,
			scale0 * w + scale1 * to1.w);
}
template<typename T>
constexpr Quat_t<T> Quat_t<T>::slerpni(const Quat_t<T> &p_to, const T &p_weight) const {
	const Quat_t<T> &from = *this;

	T dot = from.dot(p_to);

	if (abs(dot) > T(0.9999)) {
		return from;
	}

	T theta = acos(dot),
            sinT = 1 / sinf(theta),
		    newFactor = sinf(p_weight * theta) * sinT,
		    invFactor = sinf((1 - p_weight) * theta) * sinT;

	return Quat_t<T>(invFactor * from.x + newFactor * p_to.x,
			invFactor * from.y + newFactor * p_to.y,
			invFactor * from.z + newFactor * p_to.z,
			invFactor * from.w + newFactor * p_to.w);
}
template<typename T>
constexpr Quat_t<T> Quat_t<T>::cubic_slerp(const Quat_t<T> &p_b, const Quat_t<T> &p_pre_a, const Quat_t<T> &p_post_b, const T &p_weight) const {

	T t2 = (1.0 - p_weight) * p_weight * 2;
	Quat_t<T> sp = this->slerp(p_b, p_weight);
	Quat_t<T> sq = p_pre_a.slerpni(p_post_b, p_weight);
	return sp.slerpni(sq, t2);
}
template<typename T>
constexpr void Quat_t<T>::set_axis_angle(const Vector3_t<T> &axis, const T &angle) {

	T d = axis.length();
	if (d == 0) {
		set(T(0), T(0), T(0), T(0));
	} else {
		T sin_angle = sinf(angle * static_cast<T>(0.5));
		T cos_angle = cosf(angle * static_cast<T>(0.5));
		T s = sin_angle / d;
		set(axis.x * s, axis.y * s, axis.z * s,
				cos_angle);
	}
}

}

#undef set