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



// const Transform2D_t<T> Transform2D_t<T>::IDENTITY;
// const Transform2D_t<T> Transform2D_t<T>::FLIP_X = Transform2D_t(-1, 0, 0, 1, 0, 0);
// const Transform2D_t<T> Transform2D_t<T>::FLIP_Y = Transform2D_t(1, 0, 0, -1, 0, 0);

template<arithmetic T>
Transform2D_t<T>::Transform2D_t(T xx, T xy, T yx, T yy, T ox, T oy) {
	elements[0][0] = xx;
	elements[0][1] = xy;
	elements[1][0] = yx;
	elements[1][1] = yy;
	elements[2][0] = ox;
	elements[2][1] = oy;
}

template<arithmetic T>
Vector2_t<T> Transform2D_t<T>::basis_xform(const Vector2_t<T> &v) const {
	return Vector2_t<T>(
			tdotx(v),
			tdoty(v));
}

template<arithmetic T>
Vector2_t<T> Transform2D_t<T>::basis_xform_inv(const Vector2_t<T> &v) const {
	return Vector2_t<T>(
			elements[0].dot(v),
			elements[1].dot(v));
}

template<arithmetic T>
Vector2_t<T> Transform2D_t<T>::xform(const Vector2_t<T> &v) const {
	return Vector2_t<T>(
				   tdotx(v),
				   tdoty(v)) +
		   elements[2];
}

template<arithmetic T>
Vector2_t<T> Transform2D_t<T>::xform_inv(const Vector2_t<T> &p_vec) const {
	Vector2_t<T> v = p_vec - elements[2];

	return Vector2_t<T>(
			elements[0].dot(v),
			elements[1].dot(v));
}

template<arithmetic T>
Rect2_t<T> Transform2D_t<T>::xform(const Rect2_t<T> &p_rect) const {
	Vector2_t<T> x = elements[0] * p_rect.size.x;
	Vector2_t<T> y = elements[1] * p_rect.size.y;
	Vector2_t<T> position = xform(p_rect.position);

	Rect2_t<T> new_rect;
	new_rect.position = position;
	new_rect.expand_to(position + x);
	new_rect.expand_to(position + y);
	new_rect.expand_to(position + x + y);
	return new_rect;
}


template<arithmetic T>
void Transform2D_t<T>::set_rotation_and_scale(T p_rot, const Vector2_t<T> &p_scale) {
	elements[0][0] = ::cos(p_rot) * p_scale.x;
	elements[1][1] = ::cos(p_rot) * p_scale.y;
	elements[1][0] = -::sin(p_rot) * p_scale.y;
	elements[0][1] = ::sin(p_rot) * p_scale.x;
}


template<arithmetic T>
Rect2_t<T> Transform2D_t<T>::xform_inv(const Rect2_t<T> &p_rect) const {
	Vector2_t<T> ends[4] = {
		xform_inv(p_rect.position),
		xform_inv(Vector2_t<T>(p_rect.position.x, p_rect.position.y + p_rect.size.y)),
		xform_inv(Vector2_t<T>(p_rect.position.x + p_rect.size.x, p_rect.position.y + p_rect.size.y)),
		xform_inv(Vector2_t<T>(p_rect.position.x + p_rect.size.x, p_rect.position.y))
	};

	Rect2_t<T> new_rect;
	new_rect.position = ends[0];
	new_rect.expand_to(ends[1]);
	new_rect.expand_to(ends[2]);
	new_rect.expand_to(ends[3]);

	return new_rect;
}

template<arithmetic T>
void Transform2D_t<T>::invert() {
	// FIXME: this function assumes the basis is a rotation matrix, with no scaling.
	// Transform2D_t<T>::affine_inverse can handle matrices with scaling, so GDScript should eventually use that.
	std::swap(elements[0][1], elements[1][0]);
	elements[2] = basis_xform(-elements[2]);
}

template<arithmetic T>
Transform2D_t<T> Transform2D_t<T>::inverse() const {
	Transform2D_t inv = *this;
	inv.invert();
	return inv;
}

template<arithmetic T>
void Transform2D_t<T>::affine_invert() {
	T det = basis_determinant();
	ERR_FAIL_COND(det == 0);
	T idet = T(1) / det;

	std::swap(elements[0][0], elements[1][1]);
	elements[0] *= Vector2_t<T>(idet, -idet);
	elements[1] *= Vector2_t<T>(-idet, idet);

	elements[2] = basis_xform(-elements[2]);
}

template<arithmetic T>
Transform2D_t<T> Transform2D_t<T>::affine_inverse() const {
	Transform2D_t inv = *this;
	inv.affine_invert();
	return inv;
}

template<arithmetic T>
void Transform2D_t<T>::rotate(T p_phi) {
	*this = Transform2D_t(p_phi, Vector2_t<T>()) * (*this);
}

template<arithmetic T>
T Transform2D_t<T>::get_rotation() const {
	T det = basis_determinant();
	Transform2D_t m = orthonormalized();
	if (det < 0) {
		m.scale_basis(Vector2_t<T>(-1, -1));
	}
	return ::atan2(m[0].y, m[0].x);
}


template<arithmetic T>
void Transform2D_t<T>::set_rotation(T p_rot) {
	T cr = ::cos(p_rot);
	T sr = ::sin(p_rot);
	elements[0][0] = cr;
	elements[0][1] = sr;
	elements[1][0] = -sr;
	elements[1][1] = cr;
}

template<arithmetic T>
Transform2D_t<T>::Transform2D_t(T p_rot, const Vector2_t<T> &p_position) {
	T cr = ::cos(p_rot);
	T sr = ::sin(p_rot);
	elements[0][0] = cr;
	elements[0][1] = sr;
	elements[1][0] = -sr;
	elements[1][1] = cr;
	elements[2] = p_position;
}

template<arithmetic T>
Vector2_t<T> Transform2D_t<T>::get_scale() const {
	T det_sign = basis_determinant() > 0 ? 1 : -1;
	return det_sign * Vector2_t<T>(elements[0].length(), elements[1].length());
}

template<arithmetic T>
void Transform2D_t<T>::scale(const Vector2_t<T> &p_scale) {
	scale_basis(p_scale);
	elements[2] *= p_scale;
}

template<arithmetic T>
void Transform2D_t<T>::scale_basis(const Vector2_t<T> &p_scale) {
	elements[0][0] *= p_scale.x;
	elements[0][1] *= p_scale.y;
	elements[1][0] *= p_scale.x;
	elements[1][1] *= p_scale.y;
}

template<arithmetic T>
void Transform2D_t<T>::translate(T p_tx, T p_ty) {
	translate(Vector2_t<T>(p_tx, p_ty));
}

template<arithmetic T>
void Transform2D_t<T>::translate(const Vector2_t<T> &p_translation) {
	elements[2] += basis_xform(p_translation);
}

template<arithmetic T>
void Transform2D_t<T>::orthonormalize() {
	// Gram-Schmidt Process

	Vector2_t<T> x = elements[0];
	Vector2_t<T> y = elements[1];

	x.normalize();
	y = (y - x * (x.dot(y)));
	y.normalize();

	elements[0] = x;
	elements[1] = y;
}

template<arithmetic T>
Transform2D_t<T> Transform2D_t<T>::orthonormalized() const {
	Transform2D_t on = *this;
	on.orthonormalize();
	return on;
}

template<arithmetic T>
bool Transform2D_t<T>::operator==(const Transform2D_t &p_transform) const {
	for (int i = 0; i < 3; i++) {
		if (elements[i] != p_transform.elements[i])
			return false;
	}

	return true;
}

template<arithmetic T>
bool Transform2D_t<T>::operator!=(const Transform2D_t &p_transform) const {
	for (int i = 0; i < 3; i++) {
		if (elements[i] != p_transform.elements[i])
			return true;
	}

	return false;
}

template<arithmetic T>
void Transform2D_t<T>::operator*=(const Transform2D_t &p_transform) {
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
Transform2D_t<T> Transform2D_t<T>::operator*(const Transform2D_t &p_transform) const {
	Transform2D_t _t = *this;
	_t *= p_transform;
	return _t;
}

template<arithmetic T>
Transform2D_t<T> Transform2D_t<T>::scaled(const Vector2_t<T> &p_scale) const {
	Transform2D_t copy = *this;
	copy.scale(p_scale);
	return copy;
}

template<arithmetic T>
Transform2D_t<T> Transform2D_t<T>::basis_scaled(const Vector2_t<T> &p_scale) const {
	Transform2D_t copy = *this;
	copy.scale_basis(p_scale);
	return copy;
}

template<arithmetic T>
Transform2D_t<T> Transform2D_t<T>::untranslated() const {
	Transform2D_t copy = *this;
	copy.elements[2] = Vector2_t<T>();
	return copy;
}

template<arithmetic T>
Transform2D_t<T> Transform2D_t<T>::translated(const Vector2_t<T> &p_offset) const {
	Transform2D_t copy = *this;
	copy.translate(p_offset);
	return copy;
}

template<arithmetic T>
Transform2D_t<T> Transform2D_t<T>::rotated(T p_phi) const {
	Transform2D_t copy = *this;
	copy.rotate(p_phi);
	return copy;
}

template<arithmetic T>
T Transform2D_t<T>::basis_determinant() const {
	return elements[0].x * elements[1].y - elements[0].y * elements[1].x;
}

template<arithmetic T>
Transform2D_t<T> Transform2D_t<T>::interpolate_with(const Transform2D_t &p_transform, T p_c) const {
	//extract parameters
	Vector2_t<T> p1 = get_origin();
	Vector2_t<T> p2 = p_transform.get_origin();

	T r1 = get_rotation();
	T r2 = p_transform.get_rotation();

	Vector2_t<T> s1 = get_scale();
	Vector2_t<T> s2 = p_transform.get_scale();

	//slerp rotation
	Vector2_t<T> v1(::cos(r1), ::sin(r1));
	Vector2_t<T> v2(::cos(r2), ::sin(r2));

	T dot = v1.dot(v2);

	dot = (dot < -T(1)) ? -T(1) : ((dot > T(1)) ? T(1) : dot); //clamp dot to [-1,1]

	Vector2_t<T> v;

	if (dot > 0.9995) {
		v = Vector2_t<T>::linear_interpolate(v1, v2, p_c).normalized(); //linearly interpolate to avoid numerical precision issues
	} else {
		T angle = p_c * ::acos(dot);
		Vector2_t<T> v3 = (v2 - v1 * dot).normalized();
		v = v1 * ::cos(angle) + v3 * ::sin(angle);
	}

	//construct matrix
	Transform2D_t res(::atan2(v.y, v.x), Vector2_t<T>::linear_interpolate(p1, p2, p_c));
	res.scale_basis(Vector2_t<T>::linear_interpolate(s1, s2, p_c));
	return res;
}
