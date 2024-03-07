
template<typename real>
real Quat<real>::angle_to(const Quat<real> &p_to) const {
	real d = dot(p_to);
	return acosf(CLAMP(d * d * 2.0f - 1.0f, -1.0f, 1.0f));
}

template<typename real>
real Quat<real>::dot(const Quat<real> &p_q) const {
	return x * p_q.x + y * p_q.y + z * p_q.z + w * p_q.w;
}

template<typename real>
real Quat<real>::length_squared() const{
    return dot(*this);
}
// set_euler_xyz expects a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// This implementation uses XYZ convention (Z is the first rotation).
template<typename real>
void Quat<real>::set_euler_xyz(const Vector3 &p_euler) {
	real half_a1 = p_euler.x * 0.5;
	real half_a2 = p_euler.y * 0.5;
	real half_a3 = p_euler.z * 0.5;

	// R = X(a1).Y(a2).Z(a3) convention for Euler angles.
	// Conversion to Quat<real>ernion as listed in https://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19770024290.pdf (page A-2)
	// a3 is the angle of the first rotation, following the notation in this reference.

	real cos_a1 = cosf(half_a1);
	real sin_a1 = sinf(half_a1);
	real cos_a2 = cosf(half_a2);
	real sin_a2 = sinf(half_a2);
	real cos_a3 = cosf(half_a3);
	real sin_a3 = sinf(half_a3);

	set(sin_a1 * cos_a2 * cos_a3 + sin_a2 * sin_a3 * cos_a1,
			-sin_a1 * sin_a3 * cos_a2 + sin_a2 * cos_a1 * cos_a3,
			sin_a1 * sin_a2 * cos_a3 + sin_a3 * cos_a1 * cos_a2,
			-sin_a1 * sin_a2 * sin_a3 + cos_a1 * cos_a2 * cos_a3);
}

// get_euler_xyz returns a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// This implementation uses XYZ convention (Z is the first rotation).
// Vector3 Quat<real>::get_euler_xyz() const {
// 	Basis m(*this);
// 	return m.get_euler_xyz();
// }

// set_euler_yxz expects a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// This implementation uses YXZ convention (Z is the first rotation).
template<typename real>
void Quat<real>::set_euler_yxz(const Vector3 &p_euler) {
	real half_a1 = p_euler.y * 0.5;
	real half_a2 = p_euler.x * 0.5;
	real half_a3 = p_euler.z * 0.5;

	// R = Y(a1).X(a2).Z(a3) convention for Euler angles.
	// Conversion to Quat<real>ernion as listed in https://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19770024290.pdf (page A-6)
	// a3 is the angle of the first rotation, following the notation in this reference.

	real cos_a1 = cosf(half_a1);
	real sin_a1 = sinf(half_a1);
	real cos_a2 = cosf(half_a2);
	real sin_a2 = sinf(half_a2);
	real cos_a3 = cosf(half_a3);
	real sin_a3 = sinf(half_a3);

	set(sin_a1 * cos_a2 * sin_a3 + cos_a1 * sin_a2 * cos_a3,
			sin_a1 * cos_a2 * cos_a3 - cos_a1 * sin_a2 * sin_a3,
			-sin_a1 * sin_a2 * cos_a3 + cos_a1 * cos_a2 * sin_a3,
			sin_a1 * sin_a2 * sin_a3 + cos_a1 * cos_a2 * cos_a3);
}

// get_euler_yxz returns a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// This implementation uses YXZ convention (Z is the first rotation).
// Vector3 Quat<real>::get_euler_yxz() const {
// 	Basis m(*this);
// 	return m.get_euler_yxz();
// }
template<typename real>
void Quat<real>::operator*=(const Quat<real> &p_q) {
	set(w * p_q.x + x * p_q.w + y * p_q.z - z * p_q.y,
			w * p_q.y + y * p_q.w + z * p_q.x - x * p_q.z,
			w * p_q.z + z * p_q.w + x * p_q.y - y * p_q.x,
			w * p_q.w - x * p_q.x - y * p_q.y - z * p_q.z);
}
template<typename real>
Quat<real> Quat<real>::operator*(const Quat<real> &p_q) const {
	Quat<real> r = *this;
	r *= p_q;
	return r;
}
template<typename real>
bool Quat<real>::is_equal_approx(const Quat<real> & other) const {
	return is_equal_approx(x, other.x) && is_equal_approx(y, other.y) && is_equal_approx(z, other.z) && is_equal_approx(w, other.w);
}
template<typename real>
real Quat<real>::length() const {
	return sqrt(length_squared());
}
template<typename real>
void Quat<real>::normalize() {
	*this /= length();
}
template<typename real>
Quat<real> Quat<real>::normalized() const {
	return *this / length();
}
template<typename real>
bool Quat<real>::is_normalized() const {
	return is_equal_approx(length_squared(), real(1)); //use less epsilon
}
template<typename real>
Quat<real> Quat<real>::operator/(const real &s) const{
    return Quat<real>(x/s, y/s, z/s, w/s);
}
template<typename real>
Quat<real> Quat<real>::inverse() const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!is_normalized(), Quat<real>(), "The Quat<real>ernion must be normalized.");
#endif
	return Quat<real>(-x, -y, -z, w);
}
template<typename real>
Quat<real> Quat<real>::slerp(const Quat<real> &p_to, const real &p_weight) const {
	Quat<real> to1;
	real omega, cosom, sinom, scale0, scale1;

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
		scale0 = sinf((1.0 - p_weight) * omega) / sinom;
		scale1 = sinf(p_weight * omega) / sinom;
	} else {
		// "from" and "to" Quat<real>ernions are very close
		//  ... so we can do a linear interpolation
		scale0 = 1.0 - p_weight;
		scale1 = p_weight;
	}
	// calculate final values
	return Quat<real>(
			scale0 * x + scale1 * to1.x,
			scale0 * y + scale1 * to1.y,
			scale0 * z + scale1 * to1.z,
			scale0 * w + scale1 * to1.w);
}
template<typename real>
Quat<real> Quat<real>::slerpni(const Quat<real> &p_to, const real &p_weight) const {
	const Quat<real> &from = *this;

	real dot = from.dot(p_to);

	if (abs(dot) > real(0.9999)) {
		return from;
	}

	real theta = acos(dot),
            sinT = 1.0 / sinf(theta),
		    newFactor = sinf(p_weight * theta) * sinT,
		    invFactor = sinf((1.0 - p_weight) * theta) * sinT;

	return Quat<real>(invFactor * from.x + newFactor * p_to.x,
			invFactor * from.y + newFactor * p_to.y,
			invFactor * from.z + newFactor * p_to.z,
			invFactor * from.w + newFactor * p_to.w);
}
template<typename real>
Quat<real> Quat<real>::cubic_slerp(const Quat<real> &p_b, const Quat<real> &p_pre_a, const Quat<real> &p_post_b, const real &p_weight) const {

	real t2 = (1.0 - p_weight) * p_weight * 2;
	Quat<real> sp = this->slerp(p_b, p_weight);
	Quat<real> sq = p_pre_a.slerpni(p_post_b, p_weight);
	return sp.slerpni(sq, t2);
}
template<typename real>
void Quat<real>::set_axis_angle(const Vector3 &axis, const real &angle) {

	real d = axis.length();
	if (d == 0) {
		set(real(0), real(0), real(0), real(0));
	} else {
		real sin_angle = sinf(angle * 0.5);
		real cos_angle = cosf(angle * 0.5);
		real s = sin_angle / d;
		set(axis.x * s, axis.y * s, axis.z * s,
				cos_angle);
	}
}
