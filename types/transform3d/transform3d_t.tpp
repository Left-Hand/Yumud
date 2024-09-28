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


template<arithmetic T>
void Transform3D_t<T>::affine_invert() {
	basis.invert();
	origin = basis.xform(-origin);
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::affine_inverse() const {
	Transform3D_t<T> ret = *this;
	ret.affine_invert();
	return ret;
}

template<arithmetic T>
void Transform3D_t<T>::invert() {
	basis.transpose();
	origin = basis.xform(-origin);
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::inverse() const {
	// FIXME: this function assumes the basis is a rotation matrix, with no scaling.
	// Transform3D_t<T>::affine_inverse can handle matrices with scaling, so GDScript should eventually use that.
	Transform3D_t<T> ret = *this;
	ret.invert();
	return ret;
}

template<arithmetic T>
void Transform3D_t<T>::rotate(const Vector3_t<T> &p_axis, real_t p_angle) {
	*this = rotated(p_axis, p_angle);
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::rotated(const Vector3_t<T> &p_axis, real_t p_angle) const {
	// Equivalent to left multiplication
	Basis_t<T> p_basis(p_axis, p_angle);
	return Transform3D_t<T>(p_basis * basis, p_basis.xform(origin));
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::rotated_local(const Vector3_t<T> &p_axis, real_t p_angle) const {
	// Equivalent to right multiplication
	Basis_t<T> p_basis(p_axis, p_angle);
	return Transform3D_t<T>(basis * p_basis, origin);
}

template<arithmetic T>
void Transform3D_t<T>::rotate_basis(const Vector3_t<T> &p_axis, real_t p_angle) {
	basis.rotate(p_axis, p_angle);
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::looking_at(const Vector3_t<T> &p_target, const Vector3_t<T> &p_up, bool p_use_model_front) const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(origin.is_equal_approx(p_target), Transform3D_t<T>(), "The transform's origin and target can't be equal.");
#endif
	Transform3D_t<T> _t = *this;
	_t.basis = Basis_t<T>::looking_at(p_target - origin, p_up, p_use_model_front);
	return _t;
}

template<arithmetic T>
void Transform3D_t<T>::set_look_at(const Vector3_t<T> &p_eye, const Vector3_t<T> &p_target, const Vector3_t<T> &p_up, bool p_use_model_front) {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_MSG(p_eye.is_equal_approx(p_target), "The eye and target vectors can't be equal.");
#endif
	basis = Basis_t<T>::looking_at(p_target - p_eye, p_up, p_use_model_front);
	origin = p_eye;
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::interpolate_with(const Transform3D_t<T> &p_transform, real_t p_c) const {
	Transform3D_t<T> interp;

	Vector3_t<T> src_scale = basis.get_scale();
	Quat_t<T> src_rot = basis.get_rotation_quaternion();
	Vector3_t<T> src_loc = origin;

	Vector3_t<T> dst_scale = p_transform.basis.get_scale();
	Quat_t<T> dst_rot = p_transform.basis.get_rotation_quaternion();
	Vector3_t<T> dst_loc = p_transform.origin;

	interp.basis.set_quaternion_scale(src_rot.slerp(dst_rot, p_c).normalized(), src_scale.lerp(dst_scale, p_c));
	interp.origin = src_loc.lerp(dst_loc, p_c);

	return interp;
}

template<arithmetic T>
void Transform3D_t<T>::scale(const Vector3_t<T> &p_scale) {
	basis.scale(p_scale);
	origin *= p_scale;
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::scaled(const Vector3_t<T> &p_scale) const {
	// Equivalent to left multiplication
	return Transform3D_t<T>(basis.scaled(p_scale), origin * p_scale);
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::scaled_local(const Vector3_t<T> &p_scale) const {
	// Equivalent to right multiplication
	return Transform3D_t<T>(basis.scaled_local(p_scale), origin);
}

template<arithmetic T>
void Transform3D_t<T>::scale_basis(const Vector3_t<T> &p_scale) {
	basis.scale(p_scale);
}

template<arithmetic T>
void Transform3D_t<T>::translate_local(real_t p_tx, real_t p_ty, real_t p_tz) {
	translate_local(Vector3_t<T>(p_tx, p_ty, p_tz));
}

template<arithmetic T>
void Transform3D_t<T>::translate_local(const Vector3_t<T> &p_translation) {
	for (size_t i = 0; i < 3; i++) {
		origin[i] += basis[i].dot(p_translation);
	}
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::translated(const Vector3_t<T> &p_translation) const {
	// Equivalent to left multiplication
	return Transform3D_t<T>(basis, origin + p_translation);
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::translated_local(const Vector3_t<T> &p_translation) const {
	// Equivalent to right multiplication
	return Transform3D_t<T>(basis, origin + basis.xform(p_translation));
}

template<arithmetic T>
void Transform3D_t<T>::orthonormalize() {
	basis.orthonormalize();
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::orthonormalized() const {
	Transform3D_t<T> _copy = *this;
	_copy.orthonormalize();
	return _copy;
}

template<arithmetic T>
void Transform3D_t<T>::orthogonalize() {
	basis.orthogonalize();
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::orthogonalized() const {
	Transform3D_t<T> _copy = *this;
	_copy.orthogonalize();
	return _copy;
}

template<arithmetic T>
bool Transform3D_t<T>::is_equal_approx(const Transform3D_t<T> &p_transform) const {
	return basis.is_equal_approx(p_transform.basis) && origin.is_equal_approx(p_transform.origin);
}

template<arithmetic T>
bool Transform3D_t<T>::is_finite() const {
	return basis.is_finite() && origin.is_finite();
}

template<arithmetic T>
bool Transform3D_t<T>::operator==(const Transform3D_t<T> &p_transform) const {
	return (basis == p_transform.basis && origin == p_transform.origin);
}

template<arithmetic T>
bool Transform3D_t<T>::operator!=(const Transform3D_t<T> &p_transform) const {
	return (basis != p_transform.basis || origin != p_transform.origin);
}

template<arithmetic T>
void Transform3D_t<T>::operator*=(const Transform3D_t<T> &p_transform) {
	origin = xform(p_transform.origin);
	basis *= p_transform.basis;
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::operator*(const Transform3D_t<T> &p_transform) const {
	Transform3D_t<T> other = *this;
	other *= p_transform;
	return other;
}

template<arithmetic T>
void Transform3D_t<T>::operator*=(real_t p_val) {
	origin *= p_val;
	basis *= p_val;
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::operator*(real_t p_val) const {
	Transform3D_t<T> ret(*this);
	ret *= p_val;
	return ret;
}

template<arithmetic T>
void Transform3D_t<T>::operator/=(real_t p_val) {
	basis /= p_val;
	origin /= p_val;
}

template<arithmetic T>
Transform3D_t<T> Transform3D_t<T>::operator/(real_t p_val) const {
	Transform3D_t<T> ret(*this);
	ret /= p_val;
	return ret;
}

template<arithmetic T>
Transform3D_t<T>::Transform3D_t(const Basis_t<T> &p_basis, const Vector3_t<T> &p_origin) :
		basis(p_basis),
		origin(p_origin) {
}

template<arithmetic T>
Transform3D_t<T>::Transform3D_t(const Vector3_t<T> &p_x, const Vector3_t<T> &p_y, const Vector3_t<T> &p_z, const Vector3_t<T> &p_origin) :
		origin(p_origin) {
	basis.set_column(0, p_x);
	basis.set_column(1, p_y);
	basis.set_column(2, p_z);
}

template<arithmetic T>
Transform3D_t<T>::Transform3D_t(real_t p_xx, real_t p_xy, real_t p_xz, real_t p_yx, real_t p_yy, real_t p_yz, real_t p_zx, real_t p_zy, real_t p_zz, real_t p_ox, real_t p_oy, real_t p_oz) {
	basis = Basis_t<T>(p_xx, p_xy, p_xz, p_yx, p_yy, p_yz, p_zx, p_zy, p_zz);
	origin = Vector3_t<T>(p_ox, p_oy, p_oz);
}
