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

#include "types/aabb/aabb.hpp"
#include "types/basis/basis_t.hpp"
#include "types/quat/Quat_t.hpp"
#include "types/vector3/vector3_t.hpp"


template<arithmetic T>
struct Transform3D_t{
	Basis_t<T> basis;
	Vector3_t<T> origin;

	void invert();
	Transform3D_t<T> inverse() const;

	void affine_invert();
	Transform3D_t<T> affine_inverse() const;

	Transform3D_t<T> rotated(const Vector3_t<T> &p_axis, real_t p_angle) const;
	Transform3D_t<T> rotated_local(const Vector3_t<T> &p_axis, real_t p_angle) const;

	void rotate(const Vector3_t<T> &p_axis, real_t p_angle);
	void rotate_basis(const Vector3_t<T> &p_axis, real_t p_angle);

	void set_look_at(const Vector3_t<T> &p_eye, const Vector3_t<T> &p_target, const Vector3_t<T> &p_up = Vector3_t<T>(0, 1, 0), bool p_use_model_front = false);
	Transform3D_t<T> looking_at(const Vector3_t<T> &p_target, const Vector3_t<T> &p_up = Vector3_t<T>(0, 1, 0), bool p_use_model_front = false) const;

	void scale(const Vector3_t<T> &p_scale);
	Transform3D_t<T> scaled(const Vector3_t<T> &p_scale) const;
	Transform3D_t<T> scaled_local(const Vector3_t<T> &p_scale) const;
	void scale_basis(const Vector3_t<T> &p_scale);
	void translate_local(real_t p_tx, real_t p_ty, real_t p_tz);
	void translate_local(const Vector3_t<T> &p_translation);
	Transform3D_t<T> translated(const Vector3_t<T> &p_translation) const;
	Transform3D_t<T> translated_local(const Vector3_t<T> &p_translation) const;

	const Basis_t<T> &get_basis() const { return basis; }
	void set_basis(const Basis_t<T> &p_basis) { basis = p_basis; }

	const Vector3_t<T> &get_origin() const { return origin; }
	void set_origin(const Vector3_t<T> &p_origin) { origin = p_origin; }

	void orthonormalize();
	Transform3D_t<T> orthonormalized() const;
	void orthogonalize();
	Transform3D_t<T> orthogonalized() const;
	bool is_equal_approx(const Transform3D_t<T> &p_transform) const;
	bool is_finite() const;

	bool operator==(const Transform3D_t<T> &p_transform) const;
	bool operator!=(const Transform3D_t<T> &p_transform) const;

	__fast_inline Vector3_t<T> xform(const Vector3_t<T> &p_vector) const;
	__fast_inline AABB_t<T> xform(const AABB_t<T> &p_aabb) const;
	__fast_inline std::vector<Vector3_t<T>> xform(const std::vector<Vector3_t<T>> &p_array) const;

	// NOTE: These are UNSAFE with non-uniform scaling, and will produce incorrect results.
	// They use the transpose.
	// For safe inverse transforms, xform by the affine_inverse.
	__fast_inline Vector3_t<T> xform_inv(const Vector3_t<T> &p_vector) const;
	__fast_inline AABB_t<T> xform_inv(const AABB_t<T> &p_aabb) const;
	__fast_inline std::vector<Vector3_t<T>> xform_inv(const std::vector<Vector3_t<T>> &p_array) const;

	// Safe with non-uniform scaling (uses affine_inverse).
	__fast_inline Plane_t<T> xform(const Plane_t<T> &p_plane) const;
	__fast_inline Plane_t<T> xform_inv(const Plane_t<T> &p_plane) const;

	// These fast versions use precomputed affine inverse, and should be used in bottleneck areas where
	// multiple planes are to be transformed.
	__fast_inline Plane_t<T> xform_fast(const Plane_t<T> &p_plane, const Basis_t<T> &p_basis_inverse_transpose) const;
	static __fast_inline Plane_t<T> xform_inv_fast(const Plane_t<T> &p_plane, const Transform3D_t<T> &p_inverse, const Basis_t<T> &p_basis_transpose);

	void operator*=(const Transform3D_t<T> &p_transform);
	Transform3D_t<T> operator*(const Transform3D_t<T> &p_transform) const;
	void operator*=(real_t p_val);
	Transform3D_t<T> operator*(real_t p_val) const;
	void operator/=(real_t p_val);
	Transform3D_t<T> operator/(real_t p_val) const;

	Transform3D_t<T> interpolate_with(const Transform3D_t<T> &p_transform, real_t p_c) const;

	__fast_inline Transform3D_t<T> inverse_xform(const Transform3D_t<T> & other) const {
		Vector3_t<T> v = other.origin - origin;
		return Transform3D_t<T>(basis.transpose_xform(other.basis),
				basis.xform(v));
	}

	void set(real_t p_xx, real_t p_xy, real_t p_xz, real_t p_yx, real_t p_yy, real_t p_yz, real_t p_zx, real_t p_zy, real_t p_zz, real_t p_tx, real_t p_ty, real_t p_tz) {
		basis.set(p_xx, p_xy, p_xz, p_yx, p_yy, p_yz, p_zx, p_zy, p_zz);
		origin.x = p_tx;
		origin.y = p_ty;
		origin.z = p_tz;
	}

	operator String() const;

	Transform3D_t() {}
	Transform3D_t(const Basis_t<T> &p_basis, const Vector3_t<T> &p_origin = Vector3_t<T>());
	Transform3D_t(const Vector3_t<T> &p_x, const Vector3_t<T> &p_y, const Vector3_t<T> &p_z, const Vector3_t<T> &p_origin);
	Transform3D_t(real_t p_xx, real_t p_xy, real_t p_xz, real_t p_yx, real_t p_yy, real_t p_yz, real_t p_zx, real_t p_zy, real_t p_zz, real_t p_ox, real_t p_oy, real_t p_oz);
};

template<arithmetic T>
__fast_inline Vector3_t<T> Transform3D_t<T>::xform(const Vector3_t<T> &p_vector) const {
	return Vector3_t<T>(
			basis[0].dot(p_vector) + origin.x,
			basis[1].dot(p_vector) + origin.y,
			basis[2].dot(p_vector) + origin.z);
}

template<arithmetic T>
__fast_inline Vector3_t<T> Transform3D_t<T>::xform_inv(const Vector3_t<T> &p_vector) const {
	Vector3_t<T> v = p_vector - origin;

	return Vector3_t<T>(
			(basis[0][0] * v.x) + (basis[1][0] * v.y) + (basis[2][0] * v.z),
			(basis[0][1] * v.x) + (basis[1][1] * v.y) + (basis[2][1] * v.z),
			(basis[0][2] * v.x) + (basis[1][2] * v.y) + (basis[2][2] * v.z));
}

// Neither the plane regular xform or xform_inv are particularly efficient,
// as they do a basis inverse. For xforming a large number
// of planes it is better to pre-calculate the inverse transpose basis once
// and reuse it for each plane, by using the 'fast' version of the functions.


template<arithmetic T>
__fast_inline Plane_t<T> Transform3D_t<T>::xform(const Plane_t<T> &p_plane) const {
	Basis_t<T> b = basis.inverse();
	b.transpose();
	return xform_fast(p_plane, b);
}


template<arithmetic T>
__fast_inline Plane_t<T> Transform3D_t<T>::xform_inv(const Plane_t<T> &p_plane) const {
	Transform3D_t<T> inv = affine_inverse();
	Basis_t<T> basis_transpose = basis.transposed();
	return xform_inv_fast(p_plane, inv, basis_transpose);
}


template<arithmetic T>
__fast_inline AABB_t<T> Transform3D_t<T>::xform(const AABB_t<T> &p_aabb) const {
	/* https://dev.theomader.com/transform-bounding-boxes/ */
	Vector3_t<T> min = p_aabb.position;
	Vector3_t<T> max = p_aabb.position + p_aabb.size;
	Vector3_t<T> tmin, tmax;
	for (int i = 0; i < 3; i++) {
		tmin[i] = tmax[i] = origin[i];
		for (int j = 0; j < 3; j++) {
			real_t e = basis[i][j] * min[j];
			real_t f = basis[i][j] * max[j];
			if (e < f) {
				tmin[i] += e;
				tmax[i] += f;
			} else {
				tmin[i] += f;
				tmax[i] += e;
			}
		}
	}
	AABB_t<T> r_aabb;
	r_aabb.position = tmin;
	r_aabb.size = tmax - tmin;
	return r_aabb;
}


template<arithmetic T>
__fast_inline AABB_t<T> Transform3D_t<T>::xform_inv(const AABB_t<T> &p_aabb) const {
	/* define vertices */
	Vector3_t<T> vertices[8] = {
		Vector3_t<T>(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z + p_aabb.size.z),
		Vector3_t<T>(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z),
		Vector3_t<T>(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y, p_aabb.position.z + p_aabb.size.z),
		Vector3_t<T>(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y, p_aabb.position.z),
		Vector3_t<T>(p_aabb.position.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z + p_aabb.size.z),
		Vector3_t<T>(p_aabb.position.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z),
		Vector3_t<T>(p_aabb.position.x, p_aabb.position.y, p_aabb.position.z + p_aabb.size.z),
		Vector3_t<T>(p_aabb.position.x, p_aabb.position.y, p_aabb.position.z)
	};

	AABB_t<T> ret;

	ret.position = xform_inv(vertices[0]);

	for (int i = 1; i < 8; i++) {
		ret.expand_to(xform_inv(vertices[i]));
	}

	return ret;
}


template<arithmetic T>
std::vector<Vector3_t<T>> Transform3D_t<T>::xform(const std::vector<Vector3_t<T>> &p_array) const {
	std::vector<Vector3_t<T>> array;
	array.resize(p_array.size());

	for (size_t i = 0; i < p_array.size(); ++i) {
		array[i] = xform(p_array[i]);
	}

	return array;
}


template<arithmetic T>
std::vector<Vector3_t<T>> Transform3D_t<T>::xform_inv(const std::vector<Vector3_t<T>> &p_array) const {
	std::vector<Vector3_t<T>> array;
	array.resize(p_array.size());

	for (size_t i = 0; i < p_array.size(); ++i) {
		array[i] = xform_inv(p_array[i]);
	}

	return array;
}


template<arithmetic T>
__fast_inline Plane_t<T> Transform3D_t<T>::xform_fast(const Plane_t<T> &p_plane, const Basis_t<T> &p_basis_inverse_transpose) const {
	// Transform a single point on the plane.
	Vector3_t<T> point = p_plane.normal * p_plane.d;
	point = xform(point);

	// Use inverse transpose for correct normals with non-uniform scaling.
	Vector3_t<T> normal = p_basis_inverse_transpose.xform(p_plane.normal);
	normal.normalize();

	real_t d = normal.dot(point);
	return Plane_t<T>(normal, d);
}


template<arithmetic T>
__fast_inline Plane_t<T> Transform3D_t<T>::xform_inv_fast(const Plane_t<T> &p_plane, const Transform3D_t<T> &p_inverse, const Basis_t<T> &p_basis_transpose) {
	// Transform a single point on the plane.
	Vector3_t<T> point = p_plane.normal * p_plane.d;
	point = p_inverse.xform(point);

	// Note that instead of precalculating the transpose, an alternative
	// would be to use the transpose for the basis transform.
	// However that would be less SIMD friendly (requiring a swizzle).
	// So the cost is one extra precalced value in the calling code.
	// This is probably worth it, as this could be used in bottleneck areas. And
	// where it is not a bottleneck, the non-fast method is fine.

	// Use transpose for correct normals with non-uniform scaling.
	Vector3_t<T> normal = p_basis_transpose.xform(p_plane.normal);
	normal.normalize();

	real_t d = normal.dot(point);
	return Plane_t<T>(normal, d);
}


template<arithmetic T>
__inline OutputStream & operator<<(OutputStream & os, const Transform3D_t<T> & transform){
    os << "(";
	os << transform.basis << ',';
	os << transform.origin << ')';
	return os;
}

#include "transform3d_t.tpp"