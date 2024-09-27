#pragma once

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

#include "types/Vector3/vector3_t.hpp"
#include "types/quat/Quat_t.hpp"

template <arithmetic T>
class Basis_t {
public:
    
	Vector3_t<T> elements[3] = {
		Vector3_t<T>(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0)),
		Vector3_t<T>(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0)),
		Vector3_t<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1))
	};

	__fast_inline constexpr const Vector3_t<T> & operator[](size_t axis) const {
		return elements[axis];
	}
	__fast_inline constexpr Vector3_t<T> & operator[](size_t axis) {
		return elements[axis];
	}

	consteval size_t size() const {return 3;}
	__fast_inline constexpr Vector3_t<T> * begin(){return elements;}
	__fast_inline constexpr const Vector3_t<T> * begin() const {return elements;}
	__fast_inline constexpr Vector3_t<T> * end(){return elements + size();}
	__fast_inline constexpr const Vector3_t<T> * end() const {return elements + size();}

	void invert();
	void transpose();

	Basis_t<T> inverse() const;
	Basis_t<T> transposed() const;

	inline T determinant() const;

	void from_z(const Vector3_t<T>&p_z);

	inline Vector3_t<T>get_axis(size_t p_axis) const {
		// get actual Basis_t<T> axis (elements is transposed for performance)
		return Vector3_t<T>(elements[0][p_axis], elements[1][p_axis], elements[2][p_axis]);
	}
	inline void set_axis(size_t p_axis, const Vector3_t<T>&p_value) {
		// get actual Basis_t<T> axis (elements is transposed for performance)
		elements[0][p_axis] = p_value.x;
		elements[1][p_axis] = p_value.y;
		elements[2][p_axis] = p_value.z;
	}

	void rotate(const Vector3_t<T>&p_axis, T p_phi);
	Basis_t<T> rotated(const Vector3_t<T>&p_axis, T p_phi) const;

	void rotate_local(const Vector3_t<T>&p_axis, T p_phi);
	Basis_t<T> rotated_local(const Vector3_t<T>&p_axis, T p_phi) const;

	void rotate(const Vector3_t<T>&p_euler);
	Basis_t<T> rotated(const Vector3_t<T>&p_euler) const;

	void rotate(const Quat_t<T> &p_quat);
	Basis_t<T> rotated(const Quat_t<T> &p_quat) const;

	Vector3_t<T>get_rotation_euler() const;
	void get_rotation_axis_angle(Vector3_t<T>&p_axis, T &p_angle) const;
	void get_rotation_axis_angle_local(Vector3_t<T>&p_axis, T &p_angle) const;
	Quat_t<T> get_rotation_quat() const;
	Vector3_t<T>get_rotation() const { return get_rotation_euler(); };

	Vector3_t<T>rotref_posscale_decomposition(Basis_t<T> &rotref) const;

	Vector3_t<T>get_euler_xyz() const;
	void set_euler_xyz(const Vector3_t<T>&p_euler);

	Vector3_t<T>get_euler_xzy() const;
	void set_euler_xzy(const Vector3_t<T>&p_euler);

	Vector3_t<T>get_euler_yzx() const;
	void set_euler_yzx(const Vector3_t<T>&p_euler);

	Vector3_t<T>get_euler_yxz() const;
	void set_euler_yxz(const Vector3_t<T>&p_euler);

	Vector3_t<T>get_euler_zxy() const;
	void set_euler_zxy(const Vector3_t<T>&p_euler);

	Vector3_t<T>get_euler_zyx() const;
	void set_euler_zyx(const Vector3_t<T>&p_euler);

	Quat_t<T> get_quat() const;
	void set_quat(const Quat_t<T> &p_quat);

	Vector3_t<T>get_euler() const { return get_euler_yxz(); }
	void set_euler(const Vector3_t<T>&p_euler) { set_euler_yxz(p_euler); }

	void get_axis_angle(Vector3_t<T>&r_axis, T &r_angle) const;
	void set_axis_angle(const Vector3_t<T>&p_axis, T p_phi);

	void scale(const Vector3_t<T>&p_scale);
	Basis_t<T> scaled(const Vector3_t<T>&p_scale) const;

	void scale_local(const Vector3_t<T>&p_scale);
	Basis_t<T> scaled_local(const Vector3_t<T>&p_scale) const;

	Vector3_t<T>get_scale() const;
	Vector3_t<T>get_scale_abs() const;
	Vector3_t<T>get_scale_local() const;

	void set_axis_angle_scale(const Vector3_t<T>&p_axis, T p_phi, const Vector3_t<T>&p_scale);
	void set_euler_scale(const Vector3_t<T>&p_euler, const Vector3_t<T>&p_scale);
	void set_quat_scale(const Quat_t<T> &p_quat, const Vector3_t<T>&p_scale);

	// transposed dot products
	inline T tdotx(const Vector3_t<T>&v) const {
		return elements[0][0] * v[0] + elements[1][0] * v[1] + elements[2][0] * v[2];
	}
	inline T tdoty(const Vector3_t<T>&v) const {
		return elements[0][1] * v[0] + elements[1][1] * v[1] + elements[2][1] * v[2];
	}
	inline T tdotz(const Vector3_t<T>&v) const {
		return elements[0][2] * v[0] + elements[1][2] * v[1] + elements[2][2] * v[2];
	}

	bool is_equal_approx(const Basis_t<T> & other) const;
	// For complicated reasons, the second argument is always discarded. See #45062.
	bool is_equal_approx(const Basis_t<T> &a, const Basis_t<T> &b) const { return is_equal_approx(a); }
	bool is_equal_approx_ratio(const Basis_t<T> &a, const Basis_t<T> &b, T p_epsilon = CMP_EPSILON) const;

	bool operator==(const Basis_t<T> &p_matrix) const;
	bool operator!=(const Basis_t<T> &p_matrix) const;

	inline Vector3_t<T>xform(const Vector3_t<T>&p_vector) const;
	inline Vector3_t<T>xform_inv(const Vector3_t<T>&p_vector) const;
	inline void operator*=(const Basis_t<T> &p_matrix);
	inline Basis_t<T> operator*(const Basis_t<T> &p_matrix) const;
	inline void operator+=(const Basis_t<T> &p_matrix);
	inline Basis_t<T> operator+(const Basis_t<T> &p_matrix) const;
	inline void operator-=(const Basis_t<T> &p_matrix);
	inline Basis_t<T> operator-(const Basis_t<T> &p_matrix) const;
	inline void operator*=(T p_val);
	inline Basis_t<T> operator*(T p_val) const;

	size_t get_orthogonal_index() const;
	void set_orthogonal_index(size_t p_index);

	void set_diagonal(const Vector3_t<T>&p_diag);

	bool is_orthogonal() const;
	bool is_diagonal() const;
	bool is_rotation() const;

	Basis_t<T> slerp(const Basis_t<T> &p_to, const T p_weight) const;

	/* create / set */

	inline void set(T xx, T xy, T xz, T yx, T yy, T yz, T zx, T zy, T zz) {
		elements[0][0] = xx;
		elements[0][1] = xy;
		elements[0][2] = xz;
		elements[1][0] = yx;
		elements[1][1] = yy;
		elements[1][2] = yz;
		elements[2][0] = zx;
		elements[2][1] = zy;
		elements[2][2] = zz;
	}
	inline void set(const Vector3_t<T>&p_x, const Vector3_t<T>&p_y, const Vector3_t<T>&p_z) {
		set_axis(0, p_x);
		set_axis(1, p_y);
		set_axis(2, p_z);
	}
	inline Vector3_t<T>get_column(size_t i) const {
		return Vector3_t<T>(elements[0][i], elements[1][i], elements[2][i]);
	}

	inline Vector3_t<T>get_row(size_t i) const {
		return Vector3_t<T>(elements[i][0], elements[i][1], elements[i][2]);
	}
	inline Vector3_t<T>get_main_diagonal() const {
		return Vector3_t<T>(elements[0][0], elements[1][1], elements[2][2]);
	}

	inline void set_row(size_t i, const Vector3_t<T>&p_row) {
		elements[i][0] = p_row.x;
		elements[i][1] = p_row.y;
		elements[i][2] = p_row.z;
	}

	inline void set_zero() {
		elements[0].zero();
		elements[1].zero();
		elements[2].zero();
	}

	inline Basis_t<T> transpose_xform(const Basis_t<T> &m) const {
		return Basis_t<T>(
				elements[0].x * m[0].x + elements[1].x * m[1].x + elements[2].x * m[2].x,
				elements[0].x * m[0].y + elements[1].x * m[1].y + elements[2].x * m[2].y,
				elements[0].x * m[0].z + elements[1].x * m[1].z + elements[2].x * m[2].z,
				elements[0].y * m[0].x + elements[1].y * m[1].x + elements[2].y * m[2].x,
				elements[0].y * m[0].y + elements[1].y * m[1].y + elements[2].y * m[2].y,
				elements[0].y * m[0].z + elements[1].y * m[1].z + elements[2].y * m[2].z,
				elements[0].z * m[0].x + elements[1].z * m[1].x + elements[2].z * m[2].x,
				elements[0].z * m[0].y + elements[1].z * m[1].y + elements[2].z * m[2].y,
				elements[0].z * m[0].z + elements[1].z * m[1].z + elements[2].z * m[2].z);
	}

	void orthonormalize();
	Basis_t<T> orthonormalized() const;

	bool is_symmetric() const;
	Basis_t<T> diagonalize();

	operator Quat_t<T>() const { return get_quat(); }

	Basis_t(const Quat_t<T> &p_quat) { set_quat(p_quat); };
	Basis_t(const Quat_t<T> &p_quat, const Vector3_t<T>&p_scale) { set_quat_scale(p_quat, p_scale); }

	Basis_t(const Vector3_t<T>&p_euler) { set_euler(p_euler); }
	Basis_t(const Vector3_t<T>&p_euler, const Vector3_t<T>&p_scale) { set_euler_scale(p_euler, p_scale); }

	Basis_t(const Vector3_t<T>&p_axis, T p_phi) { set_axis_angle(p_axis, p_phi); }
	Basis_t(const Vector3_t<T>&p_axis, T p_phi, const Vector3_t<T>&p_scale) { set_axis_angle_scale(p_axis, p_phi, p_scale); }



	__fast_inline constexpr Basis_t<T>() {}

	__fast_inline constexpr Basis_t<T>(const T xx, const T xy,const T xz,const T yx,const T yy,const T yz,const T zx,const T zy,const T zz) {
		set(xx, xy, xz, yx, yy, yz, zx, zy, zz);
	}

	__fast_inline constexpr  Basis_t<T>(const Vector3_t<T>&row0, const Vector3_t<T>&row1, const Vector3_t<T>&row2) {
		elements[0] = row0;
		elements[1] = row1;
		elements[2] = row2;
	}
};

template<arithmetic T>
inline void Basis_t<T>::operator*=(const Basis_t<T> &p_matrix) {
	set(
			p_matrix.tdotx(elements[0]), p_matrix.tdoty(elements[0]), p_matrix.tdotz(elements[0]),
			p_matrix.tdotx(elements[1]), p_matrix.tdoty(elements[1]), p_matrix.tdotz(elements[1]),
			p_matrix.tdotx(elements[2]), p_matrix.tdoty(elements[2]), p_matrix.tdotz(elements[2]));
}
template<arithmetic T>
inline Basis_t<T> Basis_t<T>::operator*(const Basis_t<T> &p_matrix) const {
	return Basis_t<T>(
			p_matrix.tdotx(elements[0]), p_matrix.tdoty(elements[0]), p_matrix.tdotz(elements[0]),
			p_matrix.tdotx(elements[1]), p_matrix.tdoty(elements[1]), p_matrix.tdotz(elements[1]),
			p_matrix.tdotx(elements[2]), p_matrix.tdoty(elements[2]), p_matrix.tdotz(elements[2]));
}
template<arithmetic T>
inline void Basis_t<T>::operator+=(const Basis_t<T> &p_matrix) {
	elements[0] += p_matrix.elements[0];
	elements[1] += p_matrix.elements[1];
	elements[2] += p_matrix.elements[2];
}
template<arithmetic T>
inline Basis_t<T> Basis_t<T>::operator+(const Basis_t<T> &p_matrix) const {
	Basis_t<T> ret(*this);
	ret += p_matrix;
	return ret;
}
template<arithmetic T>
inline void Basis_t<T>::operator-=(const Basis_t<T> &p_matrix) {
	elements[0] -= p_matrix.elements[0];
	elements[1] -= p_matrix.elements[1];
	elements[2] -= p_matrix.elements[2];
}
template<arithmetic T>
inline Basis_t<T> Basis_t<T>::operator-(const Basis_t<T> &p_matrix) const {
	Basis_t<T> ret(*this);
	ret -= p_matrix;
	return ret;
}
template<arithmetic T>
inline void Basis_t<T>::operator*=(T p_val) {
	elements[0] *= p_val;
	elements[1] *= p_val;
	elements[2] *= p_val;
}
template<arithmetic T>
inline Basis_t<T> Basis_t<T>::operator*(T p_val) const {
	Basis_t<T> ret(*this);
	ret *= p_val;
	return ret;
}
template<arithmetic T>
Vector3_t<T> Basis_t<T>::xform(const Vector3_t<T>&p_vector) const {
	return Vector3_t<T>(
			elements[0].dot(p_vector),
			elements[1].dot(p_vector),
			elements[2].dot(p_vector));
}
template<arithmetic T>
Vector3_t<T> Basis_t<T>::xform_inv(const Vector3_t<T>&p_vector) const {
	return Vector3_t<T>(
			(elements[0][0] * p_vector.x) + (elements[1][0] * p_vector.y) + (elements[2][0] * p_vector.z),
			(elements[0][1] * p_vector.x) + (elements[1][1] * p_vector.y) + (elements[2][1] * p_vector.z),
			(elements[0][2] * p_vector.x) + (elements[1][2] * p_vector.y) + (elements[2][2] * p_vector.z));
}
template<arithmetic T>
T Basis_t<T>::determinant() const {
	return elements[0][0] * (elements[1][1] * elements[2][2] - elements[2][1] * elements[1][2]) -
			elements[1][0] * (elements[0][1] * elements[2][2] - elements[2][1] * elements[0][2]) +
			elements[2][0] * (elements[0][1] * elements[1][2] - elements[1][1] * elements[0][2]);
}

template<arithmetic T>
__inline OutputStream & operator<<(OutputStream & os, const Basis_t<T> & mat){
    os << "(";
	os << mat[0] << ',';
	os << mat[1] << ',';
	os << mat[2] << ')';
	return os;
}

#include "Basis_t.tpp"
