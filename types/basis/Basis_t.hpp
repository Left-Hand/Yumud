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

namespace ymd{

template <arithmetic T>
class Basis_t {
public:
    
	Vector3_t<T> x, y, z;

	__fast_inline constexpr const Vector3_t<T> & operator[](size_t axis) const {
		return (&x)[axis];
	}
	__fast_inline constexpr Vector3_t<T> & operator[](size_t axis) {
		return (&x)[axis];
	}

	consteval size_t size() const {return 3;}
	__fast_inline constexpr Vector3_t<T> * begin(){return (*this);}
	__fast_inline constexpr const Vector3_t<T> * begin() const {return (*this);}
	__fast_inline constexpr Vector3_t<T> * end(){return (*this) + size();}
	__fast_inline constexpr const Vector3_t<T> * end() const {return (*this) + size();}

	void invert();
	void transpose();

	Basis_t<T> inverse() const;
	Basis_t<T> transposed() const;

	inline T determinant() const;

	void from_z(const Vector3_t<T>&p_z);

	inline Vector3_t<T>get_axis(size_t p_axis) const {
		// get actual Basis_t<T> axis ((*this) is transposed for performance)
		return Vector3_t<T>((*this)[0][p_axis], (*this)[1][p_axis], (*this)[2][p_axis]);
	}
	inline void set_axis(size_t p_axis, const Vector3_t<T>&p_value) {
		// get actual Basis_t<T> axis ((*this) is transposed for performance)
		(*this)[0][p_axis] = p_value.x;
		(*this)[1][p_axis] = p_value.y;
		(*this)[2][p_axis] = p_value.z;
	}

	void rotate(const Vector3_t<T>&p_axis, T p_phi);

	// Multiplies the matrix from left by the rotation matrix: M -> R.M
	// Note that this does *not* rotate the matrix itself.
	//
	// The main use of Basis_t<T> is as Transform.Basis_t<T>, which is used a the transformation matrix
	// of 3D object. Rotate here refers to rotation of the object (which is R * ((*this))),
	// not the matrix itself (which is R * ((*this)) * R.transposed()).
	Basis_t<T> rotated(const Vector3_t<arithmetic auto> &p_axis, arithmetic auto p_phi) const {
		return Basis_t<T>(static_cast<Vector3_t<T>>(p_axis), static_cast<T>(p_phi)) * ((*this));
	}

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
		return (*this)[0][0] * v[0] + (*this)[1][0] * v[1] + (*this)[2][0] * v[2];
	}
	inline T tdoty(const Vector3_t<T>&v) const {
		return (*this)[0][1] * v[0] + (*this)[1][1] * v[1] + (*this)[2][1] * v[2];
	}
	inline T tdotz(const Vector3_t<T>&v) const {
		return (*this)[0][2] * v[0] + (*this)[1][2] * v[1] + (*this)[2][2] * v[2];
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
		(*this)[0][0] = xx;
		(*this)[0][1] = xy;
		(*this)[0][2] = xz;
		(*this)[1][0] = yx;
		(*this)[1][1] = yy;
		(*this)[1][2] = yz;
		(*this)[2][0] = zx;
		(*this)[2][1] = zy;
		(*this)[2][2] = zz;
	}
	inline void set(const Vector3_t<T>&p_x, const Vector3_t<T>&p_y, const Vector3_t<T>&p_z) {
		set_axis(0, p_x);
		set_axis(1, p_y);
		set_axis(2, p_z);
	}
	inline Vector3_t<T>get_column(size_t i) const {
		return Vector3_t<T>((*this)[0][i], (*this)[1][i], (*this)[2][i]);
	}

	inline Vector3_t<T>get_row(size_t i) const {
		return Vector3_t<T>((*this)[i][0], (*this)[i][1], (*this)[i][2]);
	}
	inline Vector3_t<T>get_main_diagonal() const {
		return Vector3_t<T>((*this)[0][0], (*this)[1][1], (*this)[2][2]);
	}

	inline void set_row(size_t i, const Vector3_t<T>&p_row) {
		(*this)[i][0] = p_row.x;
		(*this)[i][1] = p_row.y;
		(*this)[i][2] = p_row.z;
	}

	inline void set_zero() {
		(*this)[0].zero();
		(*this)[1].zero();
		(*this)[2].zero();
	}

	inline Basis_t<T> transpose_xform(const Basis_t<T> &m) const {
		return Basis_t<T>(
				(*this)[0].x * m[0].x + (*this)[1].x * m[1].x + (*this)[2].x * m[2].x,
				(*this)[0].x * m[0].y + (*this)[1].x * m[1].y + (*this)[2].x * m[2].y,
				(*this)[0].x * m[0].z + (*this)[1].x * m[1].z + (*this)[2].x * m[2].z,
				(*this)[0].y * m[0].x + (*this)[1].y * m[1].x + (*this)[2].y * m[2].x,
				(*this)[0].y * m[0].y + (*this)[1].y * m[1].y + (*this)[2].y * m[2].y,
				(*this)[0].y * m[0].z + (*this)[1].y * m[1].z + (*this)[2].y * m[2].z,
				(*this)[0].z * m[0].x + (*this)[1].z * m[1].x + (*this)[2].z * m[2].x,
				(*this)[0].z * m[0].y + (*this)[1].z * m[1].y + (*this)[2].z * m[2].y,
				(*this)[0].z * m[0].z + (*this)[1].z * m[1].z + (*this)[2].z * m[2].z);
	}

	void orthonormalize();
	Basis_t<T> orthonormalized() const;

	bool is_symmetric() const;
	Basis_t<T> diagonalize();

	operator Quat_t<T>() const { return get_quat(); }

	Basis_t(const Quat_t<auto> &p_quat) { set_quat(p_quat); };
	Basis_t(const Quat_t<auto> &p_quat, const Vector3_t<T>&p_scale) { set_quat_scale(p_quat, p_scale); }

	Basis_t(const Vector3_t<auto>&p_euler) { 
		set_euler(static_cast<T>(p_euler)); }
	Basis_t(const Vector3_t<auto>&p_euler, const Vector3_t<T>&p_scale){ 
		set_euler_scale(static_cast<T>(p_euler), static_cast<T>(p_scale)); }

	Basis_t(const Vector3_t<auto>&p_axis, arithmetic auto p_phi) { 
		set_axis_angle(static_cast<Vector3_t<T>>(p_axis), static_cast<T>(p_phi)); }

	Basis_t(const Vector3_t<auto>&p_axis, arithmetic auto p_phi, const Vector3_t<auto>&p_scale) { 
		set_axis_angle_scale(static_cast<Vector3_t<T>>(p_axis), static_cast<T>(p_phi), static_cast<T>(p_scale));}

	__fast_inline constexpr Basis_t<T>():
		x(Vector3_t<T>(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0))),
		y(Vector3_t<T>(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0))),
		z(Vector3_t<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)))
	{}

	template<arithmetic U = T>
	__fast_inline constexpr Basis_t<T>(const U xx, const U xy,const U xz,const U yx,const U yy,const U yz,const U zx,const U zy,const U zz):
		x(Vector3_t<T>(static_cast<T>(xx), static_cast<T>(xy), static_cast<T>(xz))),
		y(Vector3_t<T>(static_cast<T>(yx), static_cast<T>(yy), static_cast<T>(yz))),
		z(Vector3_t<T>(static_cast<T>(zx), static_cast<T>(zy), static_cast<T>(zz)))
	{}

	template<arithmetic U = T>
	__fast_inline constexpr  Basis_t<T>(const Vector3_t<U>&row0, const Vector3_t<U>&row1, const Vector3_t<U>&row2):
		x(row0),
		y(row1),
		z(row2)
	{}
};

template<arithmetic T>
inline void Basis_t<T>::operator*=(const Basis_t<T> &p_matrix) {
	set(
			p_matrix.tdotx((*this)[0]), p_matrix.tdoty((*this)[0]), p_matrix.tdotz((*this)[0]),
			p_matrix.tdotx((*this)[1]), p_matrix.tdoty((*this)[1]), p_matrix.tdotz((*this)[1]),
			p_matrix.tdotx((*this)[2]), p_matrix.tdoty((*this)[2]), p_matrix.tdotz((*this)[2]));
}
template<arithmetic T>
inline Basis_t<T> Basis_t<T>::operator*(const Basis_t<T> &p_matrix) const {
	return Basis_t<T>(
			p_matrix.tdotx((*this)[0]), p_matrix.tdoty((*this)[0]), p_matrix.tdotz((*this)[0]),
			p_matrix.tdotx((*this)[1]), p_matrix.tdoty((*this)[1]), p_matrix.tdotz((*this)[1]),
			// real_t(0), real_t(0), real_t(0),
			p_matrix.tdotx((*this)[2]), p_matrix.tdoty((*this)[2]), p_matrix.tdotz((*this)[2]));
}
template<arithmetic T>
inline void Basis_t<T>::operator+=(const Basis_t<T> &p_matrix) {
	(*this)[0] += p_matrix[0];
	(*this)[1] += p_matrix[1];
	(*this)[2] += p_matrix[2];
}
template<arithmetic T>
inline Basis_t<T> Basis_t<T>::operator+(const Basis_t<T> &p_matrix) const {
	Basis_t<T> ret((*this));
	ret += p_matrix;
	return ret;
}
template<arithmetic T>
inline void Basis_t<T>::operator-=(const Basis_t<T> &p_matrix) {
	(*this)[0] -= p_matrix[0];
	(*this)[1] -= p_matrix[1];
	(*this)[2] -= p_matrix[2];
}
template<arithmetic T>
inline Basis_t<T> Basis_t<T>::operator-(const Basis_t<T> &p_matrix) const {
	Basis_t<T> ret((*this));
	ret -= p_matrix;
	return ret;
}
template<arithmetic T>
inline void Basis_t<T>::operator*=(T p_val) {
	(*this)[0] *= p_val;
	(*this)[1] *= p_val;
	(*this)[2] *= p_val;
}
template<arithmetic T>
inline Basis_t<T> Basis_t<T>::operator*(T p_val) const {
	Basis_t<T> ret((*this));
	ret *= p_val;
	return ret;
}
template<arithmetic T>
Vector3_t<T> Basis_t<T>::xform(const Vector3_t<T>&p_vector) const {
	return Vector3_t<T>(
			(*this)[0].dot(p_vector),
			(*this)[1].dot(p_vector),
			(*this)[2].dot(p_vector));
}
template<arithmetic T>
Vector3_t<T> Basis_t<T>::xform_inv(const Vector3_t<T>&p_vector) const {
	return Vector3_t<T>(
			((*this)[0][0] * p_vector.x) + ((*this)[1][0] * p_vector.y) + ((*this)[2][0] * p_vector.z),
			((*this)[0][1] * p_vector.x) + ((*this)[1][1] * p_vector.y) + ((*this)[2][1] * p_vector.z),
			((*this)[0][2] * p_vector.x) + ((*this)[1][2] * p_vector.y) + ((*this)[2][2] * p_vector.z));
}
template<arithmetic T>
T Basis_t<T>::determinant() const {
	return (*this)[0][0] * ((*this)[1][1] * (*this)[2][2] - (*this)[2][1] * (*this)[1][2]) -
			(*this)[1][0] * ((*this)[0][1] * (*this)[2][2] - (*this)[2][1] * (*this)[0][2]) +
			(*this)[2][0] * ((*this)[0][1] * (*this)[1][2] - (*this)[1][1] * (*this)[0][2]);
}

template<arithmetic T>
__inline OutputStream & operator<<(OutputStream & os, const Basis_t<T> & mat){
	const auto splt = os.splitter();
    os << os.brackets<'('>();
	os << mat[0] << splt;
	os << mat[1] << splt;
	os << mat[2] << os.brackets<')'>();
	return os;
}

}

#include "Basis_t.tpp"
