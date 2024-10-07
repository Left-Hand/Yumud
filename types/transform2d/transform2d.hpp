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

#include "types/rect2/rect2_t.hpp"

template<arithmetic T>
struct Transform2D_t{
	// static const Transform2D_t IDENTITY;
	// static const Transform2D_t FLIP_X;
	// static const Transform2D_t FLIP_Y;

	// Warning #1: basis of Transform2D_t is stored differently from Basis. In terms of elements array, the basis matrix looks like "on paper":
	// M = (elements[0][0] elements[1][0])
	//     (elements[0][1] elements[1][1])
	// This is such that the columns, which can be interpreted as basis vectors of the coordinate system "painted" on the object, can be accessed as elements[i].
	// Note that this is the opposite of the indices in mathematical texts, meaning: $M_{12}$ in a math book corresponds to elements[1][0] here.
	// This requires additional care when working with explicit indices.
	// See https://en.wikipedia.org/wiki/Row-_and_column-major_order for further reading.

	// Warning #2: 2D be aware that unlike 3D code, 2D code uses a left-handed coordinate system: Y-axis points down,
	// and angle is measure from +X to +Y in a clockwise-fashion.

	Vector2_t<T> elements[3];

	inline T tdotx(const Vector2_t<T> &v) const { return elements[0][0] * v.x + elements[1][0] * v.y; }
	inline T tdoty(const Vector2_t<T> &v) const { return elements[0][1] * v.x + elements[1][1] * v.y; }

	inline const Vector2_t<T> &operator[](int p_idx) const { return elements[p_idx]; }
	inline Vector2_t<T> &operator[](int p_idx) { return elements[p_idx]; }

	inline Vector2_t<T> get_axis(int p_axis) const {
		ERR_FAIL_INDEX_V(p_axis, 3, Vector2_t<T>());
		return elements[p_axis];
	}
	inline void set_axis(int p_axis, const Vector2_t<T> &p_vec) {
		elements[p_axis] = p_vec;
	}

	void invert();
	Transform2D_t inverse() const;

	void affine_invert();
	Transform2D_t affine_inverse() const;

	void set_rotation(T p_phi);
	T get_rotation() const;
	void set_rotation_and_scale(T p_phi, const Vector2_t<T> &p_scale);
	void rotate(T p_phi);

	void scale(const Vector2_t<T> &p_scale);
	void scale_basis(const Vector2_t<T> &p_scale);
	void translate(T p_tx, T p_ty);
	void translate(const Vector2_t<T> &p_translation);

	T basis_determinant() const;

	Vector2_t<T> get_scale() const;

	inline const Vector2_t<T> &get_origin() const { return elements[2]; }
	inline void set_origin(const Vector2_t<T> &p_origin) { elements[2] = p_origin; }

	Transform2D_t scaled(const Vector2_t<T> &p_scale) const;
	Transform2D_t basis_scaled(const Vector2_t<T> &p_scale) const;
	Transform2D_t translated(const Vector2_t<T> &p_offset) const;
	Transform2D_t rotated(T p_phi) const;

	Transform2D_t untranslated() const;

	void orthonormalize();
	Transform2D_t orthonormalized() const;

	bool operator==(const Transform2D_t &p_transform) const;
	bool operator!=(const Transform2D_t &p_transform) const;

	void operator*=(const Transform2D_t &p_transform);
	Transform2D_t operator*(const Transform2D_t &p_transform) const;

	Transform2D_t interpolate_with(const Transform2D_t &p_transform, T p_c) const;

	Vector2_t<T> basis_xform(const Vector2_t<T> &p_vec) const;
	Vector2_t<T> basis_xform_inv(const Vector2_t<T> &p_vec) const;
	Vector2_t<T> xform(const Vector2_t<T> &p_vec) const;
	Vector2_t<T> xform_inv(const Vector2_t<T> &p_vec) const;
	Rect2_t<T> xform(const Rect2_t<T> &p_vec) const;
	Rect2_t<T> xform_inv(const Rect2_t<T> &p_vec) const;

	Transform2D_t(T xx, T xy, T yx, T yy, T ox, T oy);

	Transform2D_t(T p_rot, const Vector2_t<T> &p_pos);
	inline Transform2D_t() {
		elements[0][0] = T(1);
		elements[1][1] = T(1);
	}
};

#include "transform2d.tpp"