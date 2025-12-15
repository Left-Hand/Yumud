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

#pragma once

#include "algebra/regions/aabb.hpp"
#include "algebra/transforms/Basis.hpp"
#include "algebra/vectors/quat.hpp"
#include "algebra/vectors/vec3.hpp"


namespace ymd{

template<arithmetic T>
struct Transform3D{
	Basis<T> basis;
	Vec3<T> origin;

	void invert();
	Transform3D<T> inverse() const;

	void affine_invert();
	Transform3D<T> affine_inverse() const;

	Transform3D<T> rotated(const Vec3<T> &p_axis, T p_angle) const;
	Transform3D<T> rotated_local(const Vec3<T> &p_axis, T p_angle) const;

	void rotate(const Vec3<T> &p_axis, T p_angle);
	void rotate_basis(const Vec3<T> &p_axis, T p_angle);

	void set_look_at(const Vec3<T> &p_eye, const Vec3<T> &p_target, const Vec3<T> &p_up = Vec3<T>(0, 1, 0), bool p_use_model_front = false);
	Transform3D<T> looking_at(const Vec3<T> &p_target, const Vec3<T> &p_up = Vec3<T>(0, 1, 0), bool p_use_model_front = false) const;

	void scale(const Vec3<T> &p_scale);
	Transform3D<T> scaled(const Vec3<T> &p_scale) const;
	Transform3D<T> scaled_local(const Vec3<T> &p_scale) const;
	void scale_basis(const Vec3<T> &p_scale);
	void translate_local(T p_tx, T p_ty, T p_tz);
	void translate_local(const Vec3<T> &p_translation);
	Transform3D<T> translated(const Vec3<T> &p_translation) const;
	Transform3D<T> translated_local(const Vec3<T> &p_translation) const;

	const Basis<T> &get_basis() const { return basis; }
	void set_basis(const Basis<T> &p_basis) { basis = p_basis; }

	const Vec3<T> &get_origin() const { return origin; }
	void set_origin(const Vec3<T> &p_origin) { origin = p_origin; }

	void orthonormalize();
	Transform3D<T> orthonormalized() const;
	void orthogonalize();
	Transform3D<T> orthogonalized() const;
	bool is_equal_approx(const Transform3D<T> &p_transform) const;
	bool is_finite() const;

	bool operator==(const Transform3D<T> &p_transform) const;
	bool operator!=(const Transform3D<T> &p_transform) const;

	__fast_inline Vec3<T> xform(const Vec3<T> &p_vector) const;
	__fast_inline AABB<T> xform(const AABB<T> &p_aabb) const;
	__fast_inline std::vector<Vec3<T>> xform(const std::vector<Vec3<T>> &p_array) const;

	// NOTE: These are UNSAFE with non-uniform scaling, and will produce incorrect results.
	// They use the transpose.
	// For safe inverse transforms, xform by the affine_inverse.
	__fast_inline Vec3<T> xform_inv(const Vec3<T> &p_vector) const;
	__fast_inline AABB<T> xform_inv(const AABB<T> &p_aabb) const;
	__fast_inline std::vector<Vec3<T>> xform_inv(const std::vector<Vec3<T>> &p_array) const;

	// Safe with non-uniform scaling (uses affine_inverse).
	__fast_inline Plane<T> xform(const Plane<T> &p_plane) const;
	__fast_inline Plane<T> xform_inv(const Plane<T> &p_plane) const;

	// These fast versions use precomputed affine inverse, and should be used in bottleneck areas where
	// multiple planes are to be transformed.
	__fast_inline Plane<T> xform_fast(const Plane<T> &p_plane, const Basis<T> &p_basis_inverse_transpose) const;
	static __fast_inline Plane<T> xform_inv_fast(const Plane<T> &p_plane, const Transform3D<T> &p_inverse, const Basis<T> &p_basis_transpose);

	void operator*=(const Transform3D<T> &p_transform);
	Transform3D<T> operator*(const Transform3D<T> &p_transform) const;
	void operator*=(T p_val);
	Transform3D<T> operator*(T p_val) const;
	void operator/=(T p_val);
	Transform3D<T> operator/(T p_val) const;

	Transform3D<T> interpolate_with(const Transform3D<T> &p_transform, T p_c) const;

	__fast_inline Transform3D<T> inverse_xform(const Transform3D<T> & other) const {
		Vec3<T> v = other.origin - origin;
		return Transform3D<T>(basis.transpose_xform(other.basis),
				basis.xform(v));
	}

	void set(T p_xx, T p_xy, T p_xz, T p_yx, T p_yy, T p_yz, T p_zx, T p_zy, T p_zz, T p_tx, T p_ty, T p_tz) {
		basis.set(p_xx, p_xy, p_xz, p_yx, p_yy, p_yz, p_zx, p_zy, p_zz);
		origin.x = p_tx;
		origin.y = p_ty;
		origin.z = p_tz;
	}

	Transform3D() {}
	Transform3D(const Basis<T> &p_basis, const Vec3<T> &p_origin = Vec3<T>());
	Transform3D(const Vec3<T> &p_x, const Vec3<T> &p_y, const Vec3<T> &p_z, const Vec3<T> &p_origin);
	Transform3D(T p_xx, T p_xy, T p_xz, T p_yx, T p_yy, T p_yz, T p_zx, T p_zy, T p_zz, T p_ox, T p_oy, T p_oz);
};

template<arithmetic T>
__fast_inline Vec3<T> Transform3D<T>::xform(const Vec3<T> &p_vector) const {
	return Vec3<T>(
			basis[0].dot(p_vector) + origin.x,
			basis[1].dot(p_vector) + origin.y,
			basis[2].dot(p_vector) + origin.z);
}

template<arithmetic T>
__fast_inline Vec3<T> Transform3D<T>::xform_inv(const Vec3<T> &p_vector) const {
	Vec3<T> v = p_vector - origin;

	return Vec3<T>(
			(basis[0][0] * v.x) + (basis[1][0] * v.y) + (basis[2][0] * v.z),
			(basis[0][1] * v.x) + (basis[1][1] * v.y) + (basis[2][1] * v.z),
			(basis[0][2] * v.x) + (basis[1][2] * v.y) + (basis[2][2] * v.z));
}

// Neither the plane regular xform or xform_inv are particularly efficient,
// as they do a basis inverse. For xforming a large number
// of planes it is better to pre-calculate the inverse transpose basis once
// and reuse it for each plane, by using the 'fast' version of the functions.


template<arithmetic T>
__fast_inline Plane<T> Transform3D<T>::xform(const Plane<T> &p_plane) const {
	Basis<T> b = basis.inverse();
	b.transpose();
	return xform_fast(p_plane, b);
}


template<arithmetic T>
__fast_inline Plane<T> Transform3D<T>::xform_inv(const Plane<T> &p_plane) const {
	Transform3D<T> inv = affine_inverse();
	Basis<T> basis_transpose = basis.transposed();
	return xform_inv_fast(p_plane, inv, basis_transpose);
}


template<arithmetic T>
__fast_inline AABB<T> Transform3D<T>::xform(const AABB<T> &p_aabb) const {
	/* https://dev.theomader.com/transform-bounding-boxes/ */
	Vec3<T> min = p_aabb.position;
	Vec3<T> max = p_aabb.position + p_aabb.size;
	Vec3<T> tmin, tmax;
	for (int i = 0; i < 3; i++) {
		tmin[i] = tmax[i] = origin[i];
		for (int j = 0; j < 3; j++) {
			T e = basis[i][j] * min[j];
			T f = basis[i][j] * max[j];
			if (e < f) {
				tmin[i] += e;
				tmax[i] += f;
			} else {
				tmin[i] += f;
				tmax[i] += e;
			}
		}
	}
	AABB<T> r_aabb;
	r_aabb.position = tmin;
	r_aabb.size = tmax - tmin;
	return r_aabb;
}


template<arithmetic T>
__fast_inline AABB<T> Transform3D<T>::xform_inv(const AABB<T> &p_aabb) const {
	/* define vertices */
	Vec3<T> vertices[8] = {
		Vec3<T>(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z + p_aabb.size.z),
		Vec3<T>(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z),
		Vec3<T>(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y, p_aabb.position.z + p_aabb.size.z),
		Vec3<T>(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y, p_aabb.position.z),
		Vec3<T>(p_aabb.position.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z + p_aabb.size.z),
		Vec3<T>(p_aabb.position.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z),
		Vec3<T>(p_aabb.position.x, p_aabb.position.y, p_aabb.position.z + p_aabb.size.z),
		Vec3<T>(p_aabb.position.x, p_aabb.position.y, p_aabb.position.z)
	};

	AABB<T> ret;

	ret.position = xform_inv(vertices[0]);

	for (int i = 1; i < 8; i++) {
		ret.expand_to(xform_inv(vertices[i]));
	}

	return ret;
}


template<arithmetic T>
std::vector<Vec3<T>> Transform3D<T>::xform(const std::vector<Vec3<T>> &p_array) const {
	std::vector<Vec3<T>> array;
	array.resize(p_array.size());

	for (size_t i = 0; i < p_array.size(); ++i) {
		array[i] = xform(p_array[i]);
	}

	return array;
}


template<arithmetic T>
std::vector<Vec3<T>> Transform3D<T>::xform_inv(const std::vector<Vec3<T>> &p_array) const {
	std::vector<Vec3<T>> array;
	array.resize(p_array.size());

	for (size_t i = 0; i < p_array.size(); ++i) {
		array[i] = xform_inv(p_array[i]);
	}

	return array;
}


template<arithmetic T>
__fast_inline Plane<T> Transform3D<T>::xform_fast(const Plane<T> &p_plane, const Basis<T> &p_basis_inverse_transpose) const {
	// Transform a single point on the plane.
	Vec3<T> point = p_plane.normal * p_plane.d;
	point = xform(point);

	// Use inverse transpose for correct normals with non-uniform scaling.
	Vec3<T> normal = p_basis_inverse_transpose.xform(p_plane.normal);
	normal.normalize();

	T d = normal.dot(point);
	return Plane<T>(normal, d);
}


template<arithmetic T>
__fast_inline Plane<T> Transform3D<T>::xform_inv_fast(const Plane<T> &p_plane, const Transform3D<T> &p_inverse, const Basis<T> &p_basis_transpose) {
	// Transform a single point on the plane.
	Vec3<T> point = p_plane.normal * p_plane.d;
	point = p_inverse.xform(point);

	// Note that instead of precalculating the transpose, an alternative
	// would be to use the transpose for the basis transform.
	// However that would be less SIMD friendly (requiring a swizzle).
	// So the cost is one extra precalced value in the calling code.
	// This is probably worth it, as this could be used in bottleneck areas. And
	// where it is not a bottleneck, the non-fast method is fine.

	// Use transpose for correct normals with non-uniform scaling.
	Vec3<T> normal = p_basis_transpose.xform(p_plane.normal);
	normal.normalize();

	T d = normal.dot(point);
	return Plane<T>(normal, d);
}


template<arithmetic T>
__inline OutputStream & operator<<(OutputStream & os, const Transform3D<T> & transform){
    os << os.brackets<'('>();
	os << transform.basis << os.splitter();
	os << transform.origin << os.brackets<')'>();
	return os;
}

}


namespace ymd{


template<arithmetic T>
void Transform3D<T>::affine_invert() {
	basis.invert();
	origin = basis.xform(-origin);
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::affine_inverse() const {
	Transform3D<T> ret = *this;
	ret.affine_invert();
	return ret;
}

template<arithmetic T>
void Transform3D<T>::invert() {
	basis.transpose();
	origin = basis.xform(-origin);
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::inverse() const {
	// FIXME: this function assumes the basis is a rotation matrix, with no scaling.
	// Transform3D<T>::affine_inverse can handle matrices with scaling, so GDScript should eventually use that.
	Transform3D<T> ret = *this;
	ret.invert();
	return ret;
}

template<arithmetic T>
void Transform3D<T>::rotate(const Vec3<T> &p_axis, T p_angle) {
	*this = rotated(p_axis, p_angle);
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::rotated(const Vec3<T> &p_axis, T p_angle) const {
	// Equivalent to left multiplication
	Basis<T> p_basis(p_axis, p_angle);
	return Transform3D<T>(p_basis * basis, p_basis.xform(origin));
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::rotated_local(const Vec3<T> &p_axis, T p_angle) const {
	// Equivalent to right multiplication
	Basis<T> p_basis(p_axis, p_angle);
	return Transform3D<T>(basis * p_basis, origin);
}

template<arithmetic T>
void Transform3D<T>::rotate_basis(const Vec3<T> &p_axis, T p_angle) {
	basis.rotate(p_axis, p_angle);
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::looking_at(const Vec3<T> &p_target, const Vec3<T> &p_up, bool p_use_model_front) const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(origin.is_equal_approx(p_target), Transform3D<T>(), "The transform's origin and target can't be equal.");
#endif
	Transform3D<T> _t = *this;
	_t.basis = Basis<T>::looking_at(p_target - origin, p_up, p_use_model_front);
	return _t;
}

template<arithmetic T>
void Transform3D<T>::set_look_at(const Vec3<T> &p_eye, const Vec3<T> &p_target, const Vec3<T> &p_up, bool p_use_model_front) {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_MSG(p_eye.is_equal_approx(p_target), "The eye and target vectors can't be equal.");
#endif
	basis = Basis<T>::looking_at(p_target - p_eye, p_up, p_use_model_front);
	origin = p_eye;
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::interpolate_with(const Transform3D<T> &p_transform, T p_c) const {
	Transform3D<T> interp;

	Vec3<T> src_scale = basis.get_scale();
	Quat<T> src_rot = basis.get_rotation_quaternion();
	Vec3<T> src_loc = origin;

	Vec3<T> dst_scale = p_transform.basis.get_scale();
	Quat<T> dst_rot = p_transform.basis.get_rotation_quaternion();
	Vec3<T> dst_loc = p_transform.origin;

	interp.basis.set_quaternion_scale(src_rot.slerp(dst_rot, p_c).normalized(), src_scale.lerp(dst_scale, p_c));
	interp.origin = src_loc.lerp(dst_loc, p_c);

	return interp;
}

template<arithmetic T>
void Transform3D<T>::scale(const Vec3<T> &p_scale) {
	basis.scale(p_scale);
	origin *= p_scale;
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::scaled(const Vec3<T> &p_scale) const {
	// Equivalent to left multiplication
	return Transform3D<T>(basis.scaled(p_scale), origin * p_scale);
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::scaled_local(const Vec3<T> &p_scale) const {
	// Equivalent to right multiplication
	return Transform3D<T>(basis.scaled_local(p_scale), origin);
}

template<arithmetic T>
void Transform3D<T>::scale_basis(const Vec3<T> &p_scale) {
	basis.scale(p_scale);
}

template<arithmetic T>
void Transform3D<T>::translate_local(T p_tx, T p_ty, T p_tz) {
	translate_local(Vec3<T>(p_tx, p_ty, p_tz));
}

template<arithmetic T>
void Transform3D<T>::translate_local(const Vec3<T> &p_translation) {
	for (size_t i = 0; i < 3; i++) {
		origin[i] += basis[i].dot(p_translation);
	}
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::translated(const Vec3<T> &p_translation) const {
	// Equivalent to left multiplication
	return Transform3D<T>(basis, origin + p_translation);
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::translated_local(const Vec3<T> &p_translation) const {
	// Equivalent to right multiplication
	return Transform3D<T>(basis, origin + basis.xform(p_translation));
}

template<arithmetic T>
void Transform3D<T>::orthonormalize() {
	basis.orthonormalize();
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::orthonormalized() const {
	Transform3D<T> _copy = *this;
	_copy.orthonormalize();
	return _copy;
}

template<arithmetic T>
void Transform3D<T>::orthogonalize() {
	basis.orthogonalize();
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::orthogonalized() const {
	Transform3D<T> _copy = *this;
	_copy.orthogonalize();
	return _copy;
}

template<arithmetic T>
bool Transform3D<T>::is_equal_approx(const Transform3D<T> &p_transform) const {
	return basis.is_equal_approx(p_transform.basis) && origin.is_equal_approx(p_transform.origin);
}

template<arithmetic T>
bool Transform3D<T>::is_finite() const {
	return basis.is_finite() && origin.is_finite();
}

template<arithmetic T>
bool Transform3D<T>::operator==(const Transform3D<T> &p_transform) const {
	return (basis == p_transform.basis && origin == p_transform.origin);
}

template<arithmetic T>
bool Transform3D<T>::operator!=(const Transform3D<T> &p_transform) const {
	return (basis != p_transform.basis || origin != p_transform.origin);
}

template<arithmetic T>
void Transform3D<T>::operator*=(const Transform3D<T> &p_transform) {
	origin = xform(p_transform.origin);
	basis *= p_transform.basis;
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::operator*(const Transform3D<T> &p_transform) const {
	Transform3D<T> other = *this;
	other *= p_transform;
	return other;
}

template<arithmetic T>
void Transform3D<T>::operator*=(T p_val) {
	origin *= p_val;
	basis *= p_val;
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::operator*(T p_val) const {
	Transform3D<T> ret(*this);
	ret *= p_val;
	return ret;
}

template<arithmetic T>
void Transform3D<T>::operator/=(T p_val) {
	basis /= p_val;
	origin /= p_val;
}

template<arithmetic T>
Transform3D<T> Transform3D<T>::operator/(T p_val) const {
	Transform3D<T> ret(*this);
	ret /= p_val;
	return ret;
}

template<arithmetic T>
Transform3D<T>::Transform3D(const Basis<T> &p_basis, const Vec3<T> &p_origin) :
		basis(p_basis),
		origin(p_origin) {
}

template<arithmetic T>
Transform3D<T>::Transform3D(const Vec3<T> &p_x, const Vec3<T> &p_y, const Vec3<T> &p_z, const Vec3<T> &p_origin) :
		origin(p_origin) {
	basis.set_column(0, p_x);
	basis.set_column(1, p_y);
	basis.set_column(2, p_z);
}

template<arithmetic T>
Transform3D<T>::Transform3D(T p_xx, T p_xy, T p_xz, T p_yx, T p_yy, T p_yz, T p_zx, T p_zy, T p_zz, T p_ox, T p_oy, T p_oz) {
	basis = Basis<T>(p_xx, p_xy, p_xz, p_yx, p_yy, p_yz, p_zx, p_zy, p_zz);
	origin = Vec3<T>(p_ox, p_oy, p_oz);
}


}