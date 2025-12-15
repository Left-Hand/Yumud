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

#include "algebra/regions/rect2.hpp"


namespace ymd{

template<arithmetic T>
struct Transform2D{
	// static const Transform2D IDENTITY;
	// static const Transform2D FLIP_X;
	// static const Transform2D FLIP_Y;

	// Warning #1: basis of Transform2D is stored differently from Basis. In terms of elements array, the basis matrix looks like "on paper":
	// M = (elements[0][0] elements[1][0])
	//     (elements[0][1] elements[1][1])
	// This is such that the columns, which can be interpreted as basis vectors of the coordinate system "painted" on the object, can be accessed as elements[i].
	// Note that this is the opposite of the indices in mathematical texts, meaning: $M_{12}$ in a math book corresponds to elements[1][0] here.
	// This requires additional care when working with explicit indices.
	// See https://en.wikipedia.org/wiki/Row-_and_column-major_order for further reading.

	// Warning #2: 2D be aware that unlike 3D code, 2D code uses a left-handed coordinate system: Y-axis points down,
	// and angle is measure from +X to +Y in a clockwise-fashion.

	Vec2<T> elements[3];

	__fast_inline Transform2D(const T xx, const T xy, const T yx, const T yy, const T ox, const T oy);

	__fast_inline Transform2D(const T p_rot, const Vec2<T> &p_pos);
	__fast_inline Transform2D() {
		elements[0][0] = T(1);
		elements[1][1] = T(1);
	}
	
	static constexpr Transform2D<T> IDENTITY = Transform2D(1, 0, 0, 1, 0, 0);
	static constexpr Transform2D<T> FLIP_X = Transform2D(-1, 0, 0, 1, 0, 0);
	static constexpr Transform2D<T> FLIP_Y = Transform2D(1, 0, 0, -1, 0, 0);


	inline T tdotx(const Vec2<T> &v) const { return elements[0][0] * v.x + elements[1][0] * v.y; }
	inline T tdoty(const Vec2<T> &v) const { return elements[0][1] * v.x + elements[1][1] * v.y; }

	inline const Vec2<T> &operator[](int p_idx) const { return elements[p_idx]; }
	inline Vec2<T> &operator[](int p_idx) { return elements[p_idx]; }

	inline Vec2<T> get_axis(int p_axis) const {
		return elements[p_axis];
	}
	inline void set_axis(int p_axis, const Vec2<T> &p_vec) {
		elements[p_axis] = p_vec;
	}

	void invert();
	Transform2D inverse() const;

	void affine_invert();
	Transform2D affine_inverse() const;

	void set_rotation(T p_phi);
	T get_rotation() const;
	void set_rotation_and_scale(T p_phi, const Vec2<T> &p_scale);
	void rotate(T p_phi);

	void scale(const Vec2<T> &p_scale);
	void scale_basis(const Vec2<T> &p_scale);
	void translate(T p_tx, T p_ty);
	void translate(const Vec2<T> &p_translation);

	T basis_determinant() const;

	Vec2<T> get_scale() const;

	inline const Vec2<T> &get_origin() const { return elements[2]; }
	inline void set_origin(const Vec2<T> &p_origin) { elements[2] = p_origin; }

	Transform2D scaled(const Vec2<T> &p_scale) const;
	Transform2D basis_scaled(const Vec2<T> &p_scale) const;
	Transform2D translated(const Vec2<T> &p_offset) const;
	Transform2D rotated(T p_phi) const;

	Transform2D untranslated() const;

	void orthonormalize();
	Transform2D orthonormalized() const;

	bool operator==(const Transform2D &p_transform) const;
	bool operator!=(const Transform2D &p_transform) const;

	void operator*=(const Transform2D &p_transform);
	Transform2D operator*(const Transform2D &p_transform) const;

	Transform2D interpolate_with(const Transform2D &p_transform, T p_c) const;

	Vec2<T> basis_xform(const Vec2<T> &p_vec) const;
	Vec2<T> basis_xform_inv(const Vec2<T> &p_vec) const;
	Vec2<T> xform(const Vec2<T> &p_vec) const;
	Vec2<T> xform_inv(const Vec2<T> &p_vec) const;
	Rect2<T> xform(const Rect2<T> &p_vec) const;
	Rect2<T> xform_inv(const Rect2<T> &p_vec) const;
};


}


namespace ymd{
template<arithmetic T>
Transform2D<T>::Transform2D(T xx, T xy, T yx, T yy, T ox, T oy) {
	elements[0][0] = xx;
	elements[0][1] = xy;
	elements[1][0] = yx;
	elements[1][1] = yy;
	elements[2][0] = ox;
	elements[2][1] = oy;
}

template<arithmetic T>
Transform2D<T>::Transform2D(T p_rot, const Vec2<T> &p_position) {
	auto [sr, cr] = sincos(p_rot);
	elements[0][0] = cr;
	elements[0][1] = sr;
	elements[1][0] = -sr;
	elements[1][1] = cr;
	elements[2] = p_position;
}


template<arithmetic T>
Vec2<T> Transform2D<T>::basis_xform(const Vec2<T> &v) const {
	return Vec2<T>(
			tdotx(v),
			tdoty(v));
}

template<arithmetic T>
Vec2<T> Transform2D<T>::basis_xform_inv(const Vec2<T> &v) const {
	return Vec2<T>(
			elements[0].dot(v),
			elements[1].dot(v));
}

template<arithmetic T>
Vec2<T> Transform2D<T>::xform(const Vec2<T> &v) const {
	return Vec2<T>(
				   tdotx(v),
				   tdoty(v)) +
		   elements[2];
}

template<arithmetic T>
Vec2<T> Transform2D<T>::xform_inv(const Vec2<T> &p_vec) const {
	Vec2<T> v = p_vec - elements[2];

	return Vec2<T>(
			elements[0].dot(v),
			elements[1].dot(v));
}

template<arithmetic T>
Rect2<T> Transform2D<T>::xform(const Rect2<T> &p_rect) const {
	Vec2<T> x = elements[0] * p_rect.size.x;
	Vec2<T> y = elements[1] * p_rect.size.y;
	Vec2<T> position = xform(p_rect.position);

	Rect2<T> new_rect;
	new_rect.position = position;
	new_rect.expand_to(position + x);
	new_rect.expand_to(position + y);
	new_rect.expand_to(position + x + y);
	return new_rect;
}


template<arithmetic T>
void Transform2D<T>::set_rotation_and_scale(T p_rot, const Vec2<T> &p_scale) {
	auto [sr, cr] = sincos(p_rot);
	
	elements[0][0] = cr * p_scale.x;
	elements[1][1] = cr * p_scale.y;
	elements[1][0] = -sr * p_scale.y;
	elements[0][1] =  sr * p_scale.x;
}


template<arithmetic T>
Rect2<T> Transform2D<T>::xform_inv(const Rect2<T> &p_rect) const {
	Vec2<T> ends[4] = {
		xform_inv(p_rect.position),
		xform_inv(Vec2<T>(p_rect.position.x, p_rect.position.y + p_rect.size.y)),
		xform_inv(Vec2<T>(p_rect.position.x + p_rect.size.x, p_rect.position.y + p_rect.size.y)),
		xform_inv(Vec2<T>(p_rect.position.x + p_rect.size.x, p_rect.position.y))
	};

	Rect2<T> new_rect;
	new_rect.position = ends[0];
	new_rect.expand_to(ends[1]);
	new_rect.expand_to(ends[2]);
	new_rect.expand_to(ends[3]);

	return new_rect;
}

template<arithmetic T>
void Transform2D<T>::invert() {
	// FIXME: this function assumes the basis is a rotation matrix, with no scaling.
	// Transform2D<T>::affine_inverse can handle matrices with scaling, so GDScript should eventually use that.
	std::swap(elements[0][1], elements[1][0]);
	elements[2] = basis_xform(-elements[2]);
}

template<arithmetic T>
Transform2D<T> Transform2D<T>::inverse() const {
	Transform2D inv = *this;
	inv.invert();
	return inv;
}

template<arithmetic T>
void Transform2D<T>::affine_invert() {
	T det = basis_determinant();
	ERR_FAIL_COND(det == 0);
	T idet = T(1) / det;

	std::swap(elements[0][0], elements[1][1]);
	elements[0] *= Vec2<T>(idet, -idet);
	elements[1] *= Vec2<T>(-idet, idet);

	elements[2] = basis_xform(-elements[2]);
}

template<arithmetic T>
Transform2D<T> Transform2D<T>::affine_inverse() const {
	Transform2D inv = *this;
	inv.affine_invert();
	return inv;
}

template<arithmetic T>
void Transform2D<T>::rotate(T p_phi) {
	*this = Transform2D(p_phi, Vec2<T>()) * (*this);
}

template<arithmetic T>
T Transform2D<T>::get_rotation() const {
	T det = basis_determinant();
	Transform2D m = orthonormalized();
	if (det < 0) {
		m.scale_basis(Vec2<T>(-1, -1));
	}
	return ::atan2(m[0].y, m[0].x);
}


template<arithmetic T>
void Transform2D<T>::set_rotation(T p_rot) {

	auto [sr, cr] = sincos(p_rot);
	elements[0][0] = cr;
	elements[0][1] = sr;
	elements[1][0] = -sr;
	elements[1][1] = cr;
}


template<arithmetic T>
Vec2<T> Transform2D<T>::get_scale() const {
	T det_sign = basis_determinant() > 0 ? 1 : -1;
	return det_sign * Vec2<T>(elements[0].length(), elements[1].length());
}

template<arithmetic T>
void Transform2D<T>::scale(const Vec2<T> &p_scale) {
	scale_basis(p_scale);
	elements[2] *= p_scale;
}

template<arithmetic T>
void Transform2D<T>::scale_basis(const Vec2<T> &p_scale) {
	elements[0][0] *= p_scale.x;
	elements[0][1] *= p_scale.y;
	elements[1][0] *= p_scale.x;
	elements[1][1] *= p_scale.y;
}

template<arithmetic T>
void Transform2D<T>::translate(T p_tx, T p_ty) {
	translate(Vec2<T>(p_tx, p_ty));
}

template<arithmetic T>
void Transform2D<T>::translate(const Vec2<T> &p_translation) {
	elements[2] += basis_xform(p_translation);
}

template<arithmetic T>
void Transform2D<T>::orthonormalize() {
	// Gram-Schmidt Process

	Vec2<T> x = elements[0];
	Vec2<T> y = elements[1];

	x.normalize();
	y = (y - x * (x.dot(y)));
	y.normalize();

	elements[0] = x;
	elements[1] = y;
}

template<arithmetic T>
Transform2D<T> Transform2D<T>::orthonormalized() const {
	Transform2D on = *this;
	on.orthonormalize();
	return on;
}

template<arithmetic T>
bool Transform2D<T>::operator==(const Transform2D &p_transform) const {
	for (int i = 0; i < 3; i++) {
		if (elements[i] != p_transform.elements[i])
			return false;
	}

	return true;
}

template<arithmetic T>
bool Transform2D<T>::operator!=(const Transform2D &p_transform) const {
	for (int i = 0; i < 3; i++) {
		if (elements[i] != p_transform.elements[i])
			return true;
	}

	return false;
}

template<arithmetic T>
void Transform2D<T>::operator*=(const Transform2D &p_transform) {
	elements[2] = xform(p_transform.elements[2]);

	T x0, x1, y0, y1;

	x0 = tdotx(p_transform.elements[0]);
	x1 = tdoty(p_transform.elements[0]);
	y0 = tdotx(p_transform.elements[1]);
	y1 = tdoty(p_transform.elements[1]);

	elements[0][0] = x0;
	elements[0][1] = x1;
	elements[1][0] = y0;
	elements[1][1] = y1;
}

template<arithmetic T>
Transform2D<T> Transform2D<T>::operator*(const Transform2D &p_transform) const {
	Transform2D _t = *this;
	_t *= p_transform;
	return _t;
}

template<arithmetic T>
Transform2D<T> Transform2D<T>::scaled(const Vec2<T> &p_scale) const {
	Transform2D copy = *this;
	copy.scale(p_scale);
	return copy;
}

template<arithmetic T>
Transform2D<T> Transform2D<T>::basis_scaled(const Vec2<T> &p_scale) const {
	Transform2D copy = *this;
	copy.scale_basis(p_scale);
	return copy;
}

template<arithmetic T>
Transform2D<T> Transform2D<T>::untranslated() const {
	Transform2D copy = *this;
	copy.elements[2] = Vec2<T>();
	return copy;
}

template<arithmetic T>
Transform2D<T> Transform2D<T>::translated(const Vec2<T> &p_offset) const {
	Transform2D copy = *this;
	copy.translate(p_offset);
	return copy;
}

template<arithmetic T>
Transform2D<T> Transform2D<T>::rotated(T p_phi) const {
	Transform2D copy = *this;
	copy.rotate(p_phi);
	return copy;
}

template<arithmetic T>
T Transform2D<T>::basis_determinant() const {
	return elements[0].x * elements[1].y - elements[0].y * elements[1].x;
}

template<arithmetic T>
Transform2D<T> Transform2D<T>::interpolate_with(const Transform2D &p_transform, T p_c) const {
	//extract parameters
	Vec2<T> p1 = get_origin();
	Vec2<T> p2 = p_transform.get_origin();

	T r1 = get_rotation();
	T r2 = p_transform.get_rotation();

	Vec2<T> s1 = get_scale();
	Vec2<T> s2 = p_transform.get_scale();

	//slerp rotation
	auto [sin_r1, cos_r1] = sincos(r1);
	auto [sin_r2, cos_r2] = sincos(r2);
	
	Vec2<T> v1(cos_r1, sin_r1);
	Vec2<T> v2(cos_r2, sin_r2);

	T dot = v1.dot(v2);

	dot = (dot < -T(1)) ? -T(1) : ((dot > T(1)) ? T(1) : dot); //clamp dot to [-1,1]

	Vec2<T> v;

	if (dot > T(0.9995)) {
		v = Vec2<T>::linear_interpolate(v1, v2, p_c).normalized(); //linearly interpolate to avoid numerical precision issues
	} else {
		T angle = p_c * ::acos(dot);
		Vec2<T> v3 = (v2 - v1 * dot).normalized();
		
		auto [sr, cr] = sincos(angle);
		v = v1 * cr + v3 * sr;
	}

	//construct matrix
	Transform2D res(::atan2(v.y, v.x), Vec2<T>::linear_interpolate(p1, p2, p_c));
	res.scale_basis(Vec2<T>::linear_interpolate(s1, s2, p_c));
	return res;
}


}