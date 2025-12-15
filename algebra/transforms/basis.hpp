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

#include "algebra/vectors/vec3.hpp"
#include "algebra/vectors/quat.hpp"

namespace ymd{

template <arithmetic T>
class Basis {
public:
    
	Vec3<T> x, y, z;

	__fast_inline constexpr const Vec3<T> & operator[](size_t axis) const {
		return (&x)[axis];
	}
	__fast_inline constexpr Vec3<T> & operator[](size_t axis) {
		return (&x)[axis];
	}

	consteval size_t size() const {return 3;}
	__fast_inline constexpr Vec3<T> * begin(){return (&x);}
	__fast_inline constexpr const Vec3<T> * begin() const {return (&x);}
	__fast_inline constexpr Vec3<T> * end(){return (&x) + size();}
	__fast_inline constexpr const Vec3<T> * end() const {return (&x) + size();}

	void invert();
	void transpose();

	Basis<T> inverse() const;
	Basis<T> transposed() const;

	inline T determinant() const;

	void from_z(const Vec3<T>&p_z);

	inline Vec3<T>get_axis(size_t p_axis) const {
		// get actual Basis<T> axis ((*this) is transposed for performance)
		return Vec3<T>((*this)[0][p_axis], (*this)[1][p_axis], (*this)[2][p_axis]);
	}
	inline void set_axis(size_t p_axis, const Vec3<T>&p_value) {
		// get actual Basis<T> axis ((*this) is transposed for performance)
		(*this)[0][p_axis] = p_value.x;
		(*this)[1][p_axis] = p_value.y;
		(*this)[2][p_axis] = p_value.z;
	}

	void rotate(const Vec3<T>&p_axis, T p_phi);

	// Multiplies the matrix from left by the rotation matrix: M -> R.M
	// Note that this does *not* rotate the matrix itself.
	//
	// The main use of Basis<T> is as Transform.Basis<T>, which is used a the transformation matrix
	// of 3D object. Rotate here refers to rotation of the object (which is R * ((*this))),
	// not the matrix itself (which is R * ((*this)) * R.transposed()).
	Basis<T> rotated(const Vec3<arithmetic auto> &p_axis, arithmetic auto p_phi) const {
		return Basis<T>(static_cast<Vec3<T>>(p_axis), static_cast<T>(p_phi)) * ((*this));
	}

	void rotate_local(const Vec3<T>&p_axis, T p_phi);
	Basis<T> rotated_local(const Vec3<T>&p_axis, T p_phi) const;

	void rotate(const Vec3<T>&p_euler);
	Basis<T> rotated(const Vec3<T>&p_euler) const;

	void rotate(const Quat<T> &p_quat);
	Basis<T> rotated(const Quat<T> &p_quat) const;

	Vec3<T>get_rotation_euler() const;
	void get_rotation_axis_angle(Vec3<T>&p_axis, T &p_angle) const;
	void get_rotation_axis_angle_local(Vec3<T>&p_axis, T &p_angle) const;
	Quat<T> get_rotation_quat() const;
	Vec3<T>get_rotation() const { return get_rotation_euler(); };

	Vec3<T>rotref_posscale_decomposition(Basis<T> &rotref) const;

	Vec3<T>get_euler_xyz() const;
	void set_euler_xyz(const Vec3<T>&p_euler);

	Vec3<T>get_euler_xzy() const;
	void set_euler_xzy(const Vec3<T>&p_euler);

	Vec3<T>get_euler_yzx() const;
	void set_euler_yzx(const Vec3<T>&p_euler);

	Vec3<T>get_euler_yxz() const;
	void set_euler_yxz(const Vec3<T>&p_euler);

	Vec3<T>get_euler_zxy() const;
	void set_euler_zxy(const Vec3<T>&p_euler);

	Vec3<T>get_euler_zyx() const;
	void set_euler_zyx(const Vec3<T>&p_euler);

	Quat<T> get_quat() const;
	void set_quat(const Quat<T> &p_quat);

	Vec3<T>get_euler() const { return get_euler_yxz(); }
	void set_euler(const Vec3<T>&p_euler) { set_euler_yxz(p_euler); }

	void get_axis_angle(Vec3<T>&r_axis, T &r_angle) const;
	void set_axis_angle(const Vec3<T>&p_axis, T p_phi);

	void scale(const Vec3<T>&p_scale);
	Basis<T> scaled(const Vec3<T>&p_scale) const;

	void scale_local(const Vec3<T>&p_scale);
	Basis<T> scaled_local(const Vec3<T>&p_scale) const;

	Vec3<T>get_scale() const;
	Vec3<T>get_scale_abs() const;
	Vec3<T>get_scale_local() const;

	void set_axis_angle_scale(const Vec3<T>&p_axis, T p_phi, const Vec3<T>&p_scale);
	void set_euler_scale(const Vec3<T>&p_euler, const Vec3<T>&p_scale);
	void set_quat_scale(const Quat<T> &p_quat, const Vec3<T>&p_scale);

	// transposed dot products
	inline T tdotx(const Vec3<T>&v) const {
		return (*this)[0][0] * v[0] + (*this)[1][0] * v[1] + (*this)[2][0] * v[2];
	}
	inline T tdoty(const Vec3<T>&v) const {
		return (*this)[0][1] * v[0] + (*this)[1][1] * v[1] + (*this)[2][1] * v[2];
	}
	inline T tdotz(const Vec3<T>&v) const {
		return (*this)[0][2] * v[0] + (*this)[1][2] * v[1] + (*this)[2][2] * v[2];
	}

	bool is_equal_approx(const Basis<T> & other) const;
	// For complicated reasons, the second argument is always discarded. See #45062.
	bool is_equal_approx(const Basis<T> &a, const Basis<T> &b) const { return is_equal_approx(a); }
	bool is_equal_approx_ratio(const Basis<T> &a, const Basis<T> &b, T p_epsilon = CMP_EPSILON) const;

	bool operator==(const Basis<T> &p_matrix) const;
	bool operator!=(const Basis<T> &p_matrix) const;

	inline Vec3<T>xform(const Vec3<T>&p_vector) const;
	inline Vec3<T>xform_inv(const Vec3<T>&p_vector) const;
	inline void operator*=(const Basis<T> &p_matrix);
	inline Basis<T> operator*(const Basis<T> &p_matrix) const;
	inline void operator+=(const Basis<T> &p_matrix);
	inline Basis<T> operator+(const Basis<T> &p_matrix) const;
	inline void operator-=(const Basis<T> &p_matrix);
	inline Basis<T> operator-(const Basis<T> &p_matrix) const;
	inline void operator*=(T p_val);
	inline Basis<T> operator*(T p_val) const;

	size_t get_orthogonal_index() const;
	void set_orthogonal_index(size_t p_index);

	void set_diagonal(const Vec3<T>&p_diag);

	bool is_orthogonal() const;
	bool is_diagonal() const;
	bool is_rotation() const;

	Basis<T> slerp(const Basis<T> &p_to, const T p_weight) const;

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
	inline void set(const Vec3<T>&p_x, const Vec3<T>&p_y, const Vec3<T>&p_z) {
		set_axis(0, p_x);
		set_axis(1, p_y);
		set_axis(2, p_z);
	}
	inline Vec3<T>get_column(size_t i) const {
		return Vec3<T>((*this)[0][i], (*this)[1][i], (*this)[2][i]);
	}

	inline Vec3<T>get_row(size_t i) const {
		return Vec3<T>((*this)[i][0], (*this)[i][1], (*this)[i][2]);
	}
	inline Vec3<T>get_main_diagonal() const {
		return Vec3<T>((*this)[0][0], (*this)[1][1], (*this)[2][2]);
	}

	inline void set_row(size_t i, const Vec3<T>&p_row) {
		(*this)[i][0] = p_row.x;
		(*this)[i][1] = p_row.y;
		(*this)[i][2] = p_row.z;
	}

	inline void set_zero() {
		(*this)[0].zero();
		(*this)[1].zero();
		(*this)[2].zero();
	}

	inline Basis<T> transpose_xform(const Basis<T> &m) const {
		return Basis<T>(
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
	Basis<T> orthonormalized() const;

	bool is_symmetric() const;
	Basis<T> diagonalize();

	operator Quat<T>() const { return get_quat(); }

	Basis(const Quat<auto> &p_quat) { set_quat(p_quat); };
	Basis(const Quat<auto> &p_quat, const Vec3<T>&p_scale) { set_quat_scale(p_quat, p_scale); }

	Basis(const Vec3<auto>&p_euler) { 
		set_euler(static_cast<T>(p_euler)); }
	Basis(const Vec3<auto>&p_euler, const Vec3<T>&p_scale){ 
		set_euler_scale(static_cast<T>(p_euler), static_cast<T>(p_scale)); }

	Basis(const Vec3<auto>&p_axis, arithmetic auto p_phi) { 
		set_axis_angle(static_cast<Vec3<T>>(p_axis), static_cast<T>(p_phi)); }

	Basis(const Vec3<auto>&p_axis, arithmetic auto p_phi, const Vec3<auto>&p_scale) { 
		set_axis_angle_scale(static_cast<Vec3<T>>(p_axis), static_cast<T>(p_phi), static_cast<T>(p_scale));}

	__fast_inline constexpr Basis<T>():
		x(Vec3<T>(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0))),
		y(Vec3<T>(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0))),
		z(Vec3<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)))
	{}

	template<arithmetic U = T>
	__fast_inline constexpr Basis<T>(const U xx, const U xy,const U xz,const U yx,const U yy,const U yz,const U zx,const U zy,const U zz):
		x(Vec3<T>(static_cast<T>(xx), static_cast<T>(xy), static_cast<T>(xz))),
		y(Vec3<T>(static_cast<T>(yx), static_cast<T>(yy), static_cast<T>(yz))),
		z(Vec3<T>(static_cast<T>(zx), static_cast<T>(zy), static_cast<T>(zz)))
	{}

	template<arithmetic U = T>
	__fast_inline constexpr  Basis<T>(const Vec3<U>&row0, const Vec3<U>&row1, const Vec3<U>&row2):
		x(row0),
		y(row1),
		z(row2)
	{}
};

template<arithmetic T>
inline void Basis<T>::operator*=(const Basis<T> &p_matrix) {
	set(
			p_matrix.tdotx((*this)[0]), p_matrix.tdoty((*this)[0]), p_matrix.tdotz((*this)[0]),
			p_matrix.tdotx((*this)[1]), p_matrix.tdoty((*this)[1]), p_matrix.tdotz((*this)[1]),
			p_matrix.tdotx((*this)[2]), p_matrix.tdoty((*this)[2]), p_matrix.tdotz((*this)[2]));
}
template<arithmetic T>
inline Basis<T> Basis<T>::operator*(const Basis<T> &p_matrix) const {
	return Basis<T>(
			p_matrix.tdotx((*this)[0]), p_matrix.tdoty((*this)[0]), p_matrix.tdotz((*this)[0]),
			p_matrix.tdotx((*this)[1]), p_matrix.tdoty((*this)[1]), p_matrix.tdotz((*this)[1]),
			// real_t(0), real_t(0), real_t(0),
			p_matrix.tdotx((*this)[2]), p_matrix.tdoty((*this)[2]), p_matrix.tdotz((*this)[2]));
}
template<arithmetic T>
inline void Basis<T>::operator+=(const Basis<T> &p_matrix) {
	(*this)[0] += p_matrix[0];
	(*this)[1] += p_matrix[1];
	(*this)[2] += p_matrix[2];
}
template<arithmetic T>
inline Basis<T> Basis<T>::operator+(const Basis<T> &p_matrix) const {
	Basis<T> ret((*this));
	ret += p_matrix;
	return ret;
}
template<arithmetic T>
inline void Basis<T>::operator-=(const Basis<T> &p_matrix) {
	(*this)[0] -= p_matrix[0];
	(*this)[1] -= p_matrix[1];
	(*this)[2] -= p_matrix[2];
}
template<arithmetic T>
inline Basis<T> Basis<T>::operator-(const Basis<T> &p_matrix) const {
	Basis<T> ret((*this));
	ret -= p_matrix;
	return ret;
}
template<arithmetic T>
inline void Basis<T>::operator*=(T p_val) {
	(*this)[0] *= p_val;
	(*this)[1] *= p_val;
	(*this)[2] *= p_val;
}
template<arithmetic T>
inline Basis<T> Basis<T>::operator*(T p_val) const {
	Basis<T> ret((*this));
	ret *= p_val;
	return ret;
}
template<arithmetic T>
Vec3<T> Basis<T>::xform(const Vec3<T>&p_vector) const {
	return Vec3<T>(
			(*this)[0].dot(p_vector),
			(*this)[1].dot(p_vector),
			(*this)[2].dot(p_vector));
}
template<arithmetic T>
Vec3<T> Basis<T>::xform_inv(const Vec3<T>&p_vector) const {
	return Vec3<T>(
			((*this)[0][0] * p_vector.x) + ((*this)[1][0] * p_vector.y) + ((*this)[2][0] * p_vector.z),
			((*this)[0][1] * p_vector.x) + ((*this)[1][1] * p_vector.y) + ((*this)[2][1] * p_vector.z),
			((*this)[0][2] * p_vector.x) + ((*this)[1][2] * p_vector.y) + ((*this)[2][2] * p_vector.z));
}
template<arithmetic T>
T Basis<T>::determinant() const {
	return (*this)[0][0] * ((*this)[1][1] * (*this)[2][2] - (*this)[2][1] * (*this)[1][2]) -
			(*this)[1][0] * ((*this)[0][1] * (*this)[2][2] - (*this)[2][1] * (*this)[0][2]) +
			(*this)[2][0] * ((*this)[0][1] * (*this)[1][2] - (*this)[1][1] * (*this)[0][2]);
}

template<arithmetic T>
__inline OutputStream & operator<<(OutputStream & os, const Basis<T> & mat){
	const auto splt = os.splitter();
    os << os.brackets<'('>();
	os << mat[0] << splt;
	os << mat[1] << splt;
	os << mat[2] << os.brackets<')'>();
	return os;
}

}

#define SQRT12 static_cast<T>(3.464101615137754f)
#define cofac(row1, col1, row2, col2) ((*this)[row1][col1] * (*this)[row2][col2] - (*this)[row1][col2] * (*this)[row2][col1])


namespace ymd{


template<arithmetic T>
void Basis<T>::from_z(const Vec3<T> &p_z) {
	if (fabs(p_z.z) > SQRT12) {
		// choose p in y-z plane
		T a = p_z[1] * p_z[1] + p_z[2] * p_z[2];
		T k = 1 / sqrtf(a);
		(*this)[0] = Vec3<T>(0, -p_z[2] * k, p_z[1] * k);
		(*this)[1] = Vec3<T>(a * k, -p_z[0] * (*this)[0][2], p_z[0] * (*this)[0][1]);
	} else {
		// choose p in x-y plane
		T a = p_z.x * p_z.x + p_z.y * p_z.y;
		T k = 1 / sqrtf(a);
		(*this)[0] = Vec3<T>(-p_z.y * k, p_z.x * k, 0);
		(*this)[1] = Vec3<T>(-p_z.z * (*this)[0].y, p_z.z * (*this)[0].x, a * k);
	}
	(*this)[2] = p_z;
}
template<arithmetic T>
void Basis<T>::invert() {
	T co[3] = {
		cofac(1, 1, 2, 2), cofac(1, 2, 2, 0), cofac(1, 0, 2, 1)
	};
	T det = (*this)[0][0] * co[0] +
			(*this)[0][1] * co[1] +
			(*this)[0][2] * co[2];
	T s = 1 / det;

	set(co[0] * s, cofac(0, 2, 2, 1) * s, cofac(0, 1, 1, 2) * s,
			co[1] * s, cofac(0, 0, 2, 2) * s, cofac(0, 2, 1, 0) * s,
			co[2] * s, cofac(0, 1, 2, 0) * s, cofac(0, 0, 1, 1) * s);
}
template<arithmetic T>
void Basis<T>::orthonormalize() {
	// Gram-Schmidt Process

	Vec3<T> _x = get_axis(0);
	Vec3<T> _y = get_axis(1);
	Vec3<T> _z = get_axis(2);

	_x.normalize();
	_y = (_y - _x * (_x.dot(_y)));
	_y.normalize();
	_z = (_z - _x * (_x.dot(_z)) - _y * (_y.dot(_z)));
	_z.normalize();

	set_axis(0, _x);
	set_axis(1, _y);
	set_axis(2, _z);
}
template<arithmetic T>
Basis<T> Basis<T>::orthonormalized() const {
	Basis<T> c = (*this);
	c.orthonormalize();
	return c;
}
template<arithmetic T>
bool Basis<T>::is_orthogonal() const {
	Basis<T> identity;
	Basis<T> m = ((*this)) * transposed();

	return m.is_equal_approx(identity);
}
template<arithmetic T>
bool Basis<T>::is_diagonal() const {
	return (
			is_zero_approx((*this)[0][1]) && is_zero_approx((*this)[0][2]) &&
			is_zero_approx((*this)[1][0]) && is_zero_approx((*this)[1][2]) &&
			is_zero_approx((*this)[2][0]) && is_zero_approx((*this)[2][1]));
}
template<arithmetic T>
bool Basis<T>::is_rotation() const {
	return is_equal_approx_f(determinant(), 1) && is_orthogonal();
}
template<arithmetic T>
bool Basis<T>::is_symmetric() const {
	if (!is_equal_approx_f((*this)[0][1], (*this)[1][0])) {
		return false;
	}
	if (!is_equal_approx_f((*this)[0][2], (*this)[2][0])) {
		return false;
	}
	if (!is_equal_approx_f((*this)[1][2], (*this)[2][1])) {
		return false;
	}

	return true;
}
template<arithmetic T>
Basis<T> Basis<T>::diagonalize() {
//NOTE: only implemented for symmetric matrices
//with the Jacobi iterative method method

	const size_t ite_max = 1024;

	T off_matrix_norm_2 = (*this)[0][1] * (*this)[0][1] + (*this)[0][2] * (*this)[0][2] + (*this)[1][2] * (*this)[1][2];

	size_t ite = 0;
	Basis<T> acc_rot;
	while (off_matrix_norm_2 > CMP_EPSILON && ite++ < ite_max) {
		T el01_2 = (*this)[0][1] * (*this)[0][1];
		T el02_2 = (*this)[0][2] * (*this)[0][2];
		T el12_2 = (*this)[1][2] * (*this)[1][2];
		// Find the pivot element
		size_t i, j;
		if (el01_2 > el02_2) {
			if (el12_2 > el01_2) {
				i = 1;
				j = 2;
			} else {
				i = 0;
				j = 1;
			}
		} else {
			if (el12_2 > el02_2) {
				i = 1;
				j = 2;
			} else {
				i = 0;
				j = 2;
			}
		}

		// Compute the rotation angle
		T angle;
		if (is_equal_approx_f((*this)[j][j], (*this)[i][i])) {
			angle = PI / 4;
		} else {
			angle = 0.5 * atanf(2 * (*this)[i][j] / ((*this)[j][j] - (*this)[i][i]));
		}

		// Compute the rotation matrix
		Basis<T> rot;
		rot*(*this)[i][i] = rot * (*this)[j][j] = cosf(angle);
		rot*(*this)[i][j] = -(rot * (*this)[j][i] = sinf(angle));

		// Update the off matrix norm
		off_matrix_norm_2 -= (*this)[i][j] * (*this)[i][j];

		// Apply the rotation
		(*this) = rot * (*this) * rot.transposed();
		acc_rot = rot * acc_rot;
	}

	return acc_rot;
}
template<arithmetic T>
Basis<T> Basis<T>::inverse() const {
	Basis<T> inv = (*this);
	inv.invert();
	return inv;
}
template<arithmetic T>
void Basis<T>::transpose() {
	std::swap((*this)[0][1], (*this)[1][0]);
	std::swap((*this)[0][2], (*this)[2][0]);
	std::swap((*this)[1][2], (*this)[2][1]);
}
template<arithmetic T>
Basis<T> Basis<T>::transposed() const {
	Basis<T> tr = (*this);
	tr.transpose();
	return tr;
}

// Multiplies the matrix from left by the scaling matrix: M -> S.M
// See the comment for Basis<T>::rotated for further explanation.
template<arithmetic T>
void Basis<T>::scale(const Vec3<T> &p_scale) {
	(*this)[0][0] *= p_scale.x;
	(*this)[0][1] *= p_scale.x;
	(*this)[0][2] *= p_scale.x;
	(*this)[1][0] *= p_scale.y;
	(*this)[1][1] *= p_scale.y;
	(*this)[1][2] *= p_scale.y;
	(*this)[2][0] *= p_scale.z;
	(*this)[2][1] *= p_scale.z;
	(*this)[2][2] *= p_scale.z;
}
template<arithmetic T>
Basis<T> Basis<T>::scaled(const Vec3<T> &p_scale) const {
	Basis<T> m = (*this);
	m.scale(p_scale);
	return m;
}
template<arithmetic T>
void Basis<T>::scale_local(const Vec3<T> &p_scale) {
	// performs a scaling in object-local coordinate system:
	// M -> (M.S.Minv).M = M.S.
	(*this) = scaled_local(p_scale);
}
template<arithmetic T>
Basis<T> Basis<T>::scaled_local(const Vec3<T> &p_scale) const {
	Basis<T> b;
	b.set_diagonal(p_scale);

	return ((*this)) * b;
}
template<arithmetic T>
Vec3<T> Basis<T>::get_scale_abs() const {
	return Vec3<T>(
			Vec3<T>((*this)[0][0], (*this)[1][0], (*this)[2][0]).length(),
			Vec3<T>((*this)[0][1], (*this)[1][1], (*this)[2][1]).length(),
			Vec3<T>((*this)[0][2], (*this)[1][2], (*this)[2][2]).length());
}
template<arithmetic T>
Vec3<T> Basis<T>::get_scale_local() const {
	T det_sign = SGN(determinant());
	return Vec3<T>((*this)[0].length(), (*this)[1].length(), (*this)[2].length()) * det_sign;
}

// get_scale works with get_rotation, use get_scale_abs if you need to enforce positive signature.
template<arithmetic T>
Vec3<T> Basis<T>::get_scale() const {
	// FIXME: We are assuming M = R.S (R is rotation and S is scaling), and use polar decomposition to extract R and S.
	// A polar decomposition is M = O.P, where O is an orthogonal matrix (meaning rotation and reflection) and
	// P is a positive semi-definite matrix (meaning it contains absolute values of scaling along its diagonal).
	//
	// Despite being different from what we want to achieve, we can nevertheless make use of polar decomposition
	// here as follows. We can split O into a rotation and a reflection as O = R.Q, and obtain M = R.S where
	// we defined S = Q.P. Now, R is a proper rotation matrix and S is a (signed) scaling matrix,
	// which can involve negative scalings. However, there is a catch: unlike the polar decomposition of M = O.P,
	// the decomposition of O into a rotation and reflection matrix as O = R.Q is not unique.
	// Therefore, we are going to do this decomposition by sticking to a particular convention.
	// This may lead to confusion for some users though.
	//
	// The convention we use here is to absorb the sign flip into the scaling matrix.
	// The same convention is also used in other similar functions such as get_rotation_axis_angle, get_rotation, ...
	//
	// A proper way to get rid of this issue would be to store the scaling values (or at least their signs)
	// as a part of Basis<T>. However, if we go that path, we need to disable direct (write) access to the
	// matrix (*this).
	//
	// The rotation part of this decomposition is returned by get_rotation* functions.
	T det_sign = SGN(determinant());
	return get_scale_abs() * det_sign;
}

// Decomposes a Basis<T> into a rotation-reflection matrix (an element of the group O(3)) and a positive scaling matrix as B = O.S.
// Returns the rotation-reflection matrix via reference argument, and scaling information is returned as a Vec3<T>.
// This (internal) function is too specific and named too ugly to expose to users, and probably there's no need to do so.
template<arithmetic T>
Vec3<T> Basis<T>::rotref_posscale_decomposition(Basis<T> &rotref) const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V(determinant() == 0, Vec3<T>());

	Basis<T> m = transposed() * ((*this));
	ERR_FAIL_COND_V(!m.is_diagonal(), Vec3<T>());
#endif
	Vec3<T> scale = get_scale();
	Basis<T> inv_scale = Basis<T>().scaled(scale.inverse()); // this will also absorb the sign of scale
	rotref = ((*this)) * inv_scale;

#ifdef MATH_CHECKS
	ERR_FAIL_COND_V(!rotref.is_orthogonal(), Vec3<T>());
#endif
	return scale.abs();
}



template<arithmetic T>
void Basis<T>::rotate(const Vec3<T> &p_axis, T p_phi) {
	(*this) = rotated(p_axis, p_phi);
}
template<arithmetic T>
void Basis<T>::rotate_local(const Vec3<T> &p_axis, T p_phi) {
	// performs a rotation in object-local coordinate system:
	// M -> (M.R.Minv).M = M.R.
	(*this) = rotated_local(p_axis, p_phi);
}
template<arithmetic T>
Basis<T> Basis<T>::rotated_local(const Vec3<T> &p_axis, T p_phi) const {
	return ((*this)) * Basis<T>(p_axis, p_phi);
}
template<arithmetic T>
Basis<T> Basis<T>::rotated(const Vec3<T> &p_euler) const {
	return Basis<T>(p_euler) * ((*this));
}
template<arithmetic T>
void Basis<T>::rotate(const Vec3<T> &p_euler) {
	(*this) = rotated(p_euler);
}
template<arithmetic T>
Basis<T> Basis<T>::rotated(const Quat<T> &p_quat) const {
	return Basis<T>(p_quat) * ((*this));
}
template<arithmetic T>
void Basis<T>::rotate(const Quat<T> &p_quat) {
	(*this) = rotated(p_quat);
}
template<arithmetic T>
Vec3<T> Basis<T>::get_rotation_euler() const {
	// Assumes that the matrix can be decomposed into a proper rotation and scaling matrix as M = R.S,
	// and returns the Euler angles corresponding to the rotation part, complementing get_scale().
	// See the comment in get_scale() for further information.
	Basis<T> m = orthonormalized();
	T det = m.determinant();
	if (det < 0) {
		// Ensure that the determinant is 1, such that result is a proper rotation matrix which can be represented by Euler angles.
		m.scale(Vec3<T>(-1, -1, -1));
	}

	return m.get_euler();
}
template<arithmetic T>
Quat<T> Basis<T>::get_rotation_quat() const {
	// Assumes that the matrix can be decomposed into a proper rotation and scaling matrix as M = R.S,
	// and returns the Euler angles corresponding to the rotation part, complementing get_scale().
	// See the comment in get_scale() for further information.
	Basis<T> m = orthonormalized();
	T det = m.determinant();
	if (det < 0) {
		// Ensure that the determinant is 1, such that result is a proper rotation matrix which can be represented by Euler angles.
		m.scale(Vec3<T>(-1, -1, -1));
	}

	return m.get_quat();
}
template<arithmetic T>
void Basis<T>::get_rotation_axis_angle(Vec3<T> &p_axis, T &p_angle) const {
	// Assumes that the matrix can be decomposed into a proper rotation and scaling matrix as M = R.S,
	// and returns the Euler angles corresponding to the rotation part, complementing get_scale().
	// See the comment in get_scale() for further information.
	Basis<T> m = orthonormalized();
	T det = m.determinant();
	if (det < 0) {
		// Ensure that the determinant is 1, such that result is a proper rotation matrix which can be represented by Euler angles.
		m.scale(Vec3<T>(-1, -1, -1));
	}

	m.get_axis_angle(p_axis, p_angle);
}
template<arithmetic T>
void Basis<T>::get_rotation_axis_angle_local(Vec3<T> &p_axis, T &p_angle) const {
	// Assumes that the matrix can be decomposed into a proper rotation and scaling matrix as M = R.S,
	// and returns the Euler angles corresponding to the rotation part, complementing get_scale().
	// See the comment in get_scale() for further information.
	Basis<T> m = transposed();
	m.orthonormalize();
	T det = m.determinant();
	if (det < 0) {
		// Ensure that the determinant is 1, such that result is a proper rotation matrix which can be represented by Euler angles.
		m.scale(Vec3<T>(-1, -1, -1));
	}

	m.get_axis_angle(p_axis, p_angle);
	p_angle = -p_angle;
}

// get_euler_xyz returns a vector containing the Euler angles in the format
// (a1,a2,a3), where a3 is the angle of the first rotation, and a1 is the last
// (following the convention they are commonly defined in the literature).
//
// The current implementation uses XYZ convention (Z is the first rotation),
// so euler.z is the angle of the (first) rotation around Z axis and so on,
//
// And thus, assuming the matrix is a rotation matrix, this function returns
// the angles in the decomposition R = X(a1).Y(a2).Z(a3) where Z(a) rotates
// around the z-axis by a and so on.
template<arithmetic T>
Vec3<T> Basis<T>::get_euler_xyz() const {
	// Euler angles in XYZ convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cy*cz          -cy*sz           sy
	//        cz*sx*sy+cx*sz  cx*cz-sx*sy*sz -cy*sx
	//       -cx*cz*sy+sx*sz  cz*sx+cx*sy*sz  cx*cy

	Vec3<T> euler;
	T sy = (*this)[0][2];
	if (sy < (T(1 - CMP_EPSILON))) {
		if (sy > -(T(1 - CMP_EPSILON))) {
			// is this a pure Y rotation?
			if ((*this)[1][0] == 0 && (*this)[0][1] == 0 && (*this)[1][2] == 0 && (*this)[2][1] == 0 && (*this)[1][1] == 1) {
				// return the simplest form (human friendlier in editor and scripts)
				euler.x = 0;
				euler.y = atan2((*this)[0][2], (*this)[0][0]);
				euler.z = 0;
			} else {
				euler.x = atan2f(-(*this)[1][2], (*this)[2][2]);
				euler.y = asinf(sy);
				euler.z = atan2f(-(*this)[0][1], (*this)[0][0]);
			}
		} else {
			euler.x = atan2f((*this)[2][1], (*this)[1][1]);
			euler.y = -T(PI / 2.0);
			euler.z = 0;
		}
	} else {
		euler.x = atan2f((*this)[2][1], (*this)[1][1]);
		euler.y = T(PI / 2.0);
		euler.z = 0;
	}
	return euler;
}

// set_euler_xyz expects a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// The current implementation uses XYZ convention (Z is the first rotation).
template<arithmetic T>
void Basis<T>::set_euler_xyz(const Vec3<T> &p_euler) {

	T c, s;

	std::tie(c,s) = sincos(p_euler.x);
	Basis<T> xmat(1, 0, 0, 0, c, -s, 0, s, c);

	std::tie(c,s) = sincos(p_euler.y);
	Basis<T> ymat(c, 0, s, 0, 1, 0, -s, 0, c);

	std::tie(c,s) = sincos(p_euler.z);
	Basis<T> zmat(c, -s, 0, s, c, 0, 0, 0, 1);

	//optimizer will optimize away all this anyway
	(*this) = xmat * (ymat * zmat);
}
template<arithmetic T>
Vec3<T> Basis<T>::get_euler_xzy() const {
	// Euler angles in XZY convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cz*cy             -sz             cz*sy
	//        sx*sy+cx*cy*sz    cx*cz           cx*sz*sy-cy*sx
	//        cy*sx*sz          cz*sx           cx*cy+sx*sz*sy

	Vec3<T> euler;
	T sz = (*this)[0][1];
	if (sz < (1 - CMP_EPSILON)) {
		if (sz > -(1 - CMP_EPSILON)) {
			euler.x = atan2f((*this)[2][1], (*this)[1][1]);
			euler.y = atan2f((*this)[0][2], (*this)[0][0]);
			euler.z = asinf(-sz);
		} else {
			// It's -1
			euler.x = -atan2f((*this)[1][2], (*this)[2][2]);
			euler.y = 0;
			euler.z = PI / 2.0;
		}
	} else {
		// It's 1
		euler.x = -atan2f((*this)[1][2], (*this)[2][2]);
		euler.y = 0;
		euler.z = -PI / 2.0;
	}
	return euler;
}
template<arithmetic T>
void Basis<T>::set_euler_xzy(const Vec3<T> &p_euler) {
	T c, s;

	std::tie(c,s) = sincos(p_euler.x);
	Basis<T> xmat(1, 0, 0, 0, c, -s, 0, s, c);

	std::tie(c,s) = sincos(p_euler.y);
	Basis<T> ymat(c, 0, s, 0, 1, 0, -s, 0, c);

	std::tie(c,s) = sincos(p_euler.z);
	Basis<T> zmat(c, -s, 0, s, c, 0, 0, 0, 1);
	(*this) = xmat * zmat * ymat;
}
template<arithmetic T>
Vec3<T> Basis<T>::get_euler_yzx() const {
	// Euler angles in YZX convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cy*cz             sy*sx-cy*cx*sz     cx*sy+cy*sz*sx
	//        sz                cz*cx              -cz*sx
	//        -cz*sy            cy*sx+cx*sy*sz     cy*cx-sy*sz*sx

	Vec3<T> euler;
	T sz = (*this)[1][0];
	if (sz < (1 - CMP_EPSILON)) {
		if (sz > -(1 - CMP_EPSILON)) {
			euler.x = atan2f(-(*this)[1][2], (*this)[1][1]);
			euler.y = atan2f(-(*this)[2][0], (*this)[0][0]);
			euler.z = asinf(sz);
		} else {
			// It's -1
			euler.x = atan2f((*this)[2][1], (*this)[2][2]);
			euler.y = 0;
			euler.z = -PI / 2.0;
		}
	} else {
		// It's 1
		euler.x = atan2f((*this)[2][1], (*this)[2][2]);
		euler.y = 0;
		euler.z = PI / 2.0;
	}
	return euler;
}
template<arithmetic T>
void Basis<T>::set_euler_yzx(const Vec3<T> &p_euler) {
	T c, s;

	std::tie(c,s) = sincos(p_euler.x);
	Basis<T> xmat(1, 0, 0, 0, c, -s, 0, s, c);

	std::tie(c,s) = sincos(p_euler.y);
	Basis<T> ymat(c, 0, s, 0, 1, 0, -s, 0, c);

	std::tie(c,s) = sincos(p_euler.z);
	Basis<T> zmat(c, -s, 0, s, c, 0, 0, 0, 1);
	(*this) = ymat * zmat * xmat;
}

// get_euler_yxz returns a vector containing the Euler angles in the YXZ convention,
// as in first-Z, then-X, last-Y. The angles for X, Y, and Z rotations are returned
// as the x, y, and z components of a Vec3<T> respectively.
template<arithmetic T>
Vec3<T> Basis<T>::get_euler_yxz() const {
	// Euler angles in YXZ convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cy*cz+sy*sx*sz    cz*sy*sx-cy*sz        cx*sy
	//        cx*sz             cx*cz                 -sx
	//        cy*sx*sz-cz*sy    cy*cz*sx+sy*sz        cy*cx

	Vec3<T> euler;

	T m12 = (*this)[1][2];

	if (m12 < (1 - CMP_EPSILON)) {
		if (m12 > -(1 - CMP_EPSILON)) {
			// is this a pure X rotation?
			if ((*this)[1][0] == 0 && (*this)[0][1] == 0 && (*this)[0][2] == 0 && (*this)[2][0] == 0 && (*this)[0][0] == 1) {
				// return the simplest form (human friendlier in editor and scripts)
				euler.x = atan2(-m12, (*this)[1][1]);
				euler.y = 0;
				euler.z = 0;
			} else {
				euler.x = asin(-m12);
				euler.y = atan2((*this)[0][2], (*this)[2][2]);
				euler.z = atan2((*this)[1][0], (*this)[1][1]);
			}
		} else { // m12 == -1
			euler.x = PI * 0.5;
			euler.y = atan2((*this)[0][1], (*this)[0][0]);
			euler.z = 0;
		}
	} else { // m12 == 1
		euler.x = -PI * 0.5;
		euler.y = -atan2((*this)[0][1], (*this)[0][0]);
		euler.z = 0;
	}

	return euler;
}

// set_euler_yxz expects a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// The current implementation uses YXZ convention (Z is the first rotation).
template<arithmetic T>
void Basis<T>::set_euler_yxz(const Vec3<T> &p_euler) {
	T c, s;

	std::tie(c,s) = sincos(p_euler.x);
	Basis<T> xmat(1, 0, 0, 0, c, -s, 0, s, c);

	std::tie(c,s) = sincos(p_euler.y);
	Basis<T> ymat(c, 0, s, 0, 1, 0, -s, 0, c);

	std::tie(c,s) = sincos(p_euler.z);
	Basis<T> zmat(c, -s, 0, s, c, 0, 0, 0, 1);

	//optimizer will optimize away all this anyway
	(*this) = ymat * xmat * zmat;
}
template<arithmetic T>
Vec3<T> Basis<T>::get_euler_zxy() const {
	// Euler angles in ZXY convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cz*cy-sz*sx*sy    -cx*sz                cz*sy+cy*sz*sx
	//        cy*sz+cz*sx*sy    cz*cx                 sz*sy-cz*cy*sx
	//        -cx*sy            sx                    cx*cy
	Vec3<T> euler;
	T sx = (*this)[2][1];
	if (sx < (1 - CMP_EPSILON)) {
		if (sx > -(1 - CMP_EPSILON)) {
			euler.x = asinf(sx);
			euler.y = atan2f(-(*this)[2][0], (*this)[2][2]);
			euler.z = atan2f(-(*this)[0][1], (*this)[1][1]);
		} else {
			// It's -1
			euler.x = -PI / 2.0;
			euler.y = atan2f((*this)[0][2], (*this)[0][0]);
			euler.z = 0;
		}
	} else {
		// It's 1
		euler.x = PI / 2.0;
		euler.y = atan2f((*this)[0][2], (*this)[0][0]);
		euler.z = 0;
	}
	return euler;
}
template<arithmetic T>
void Basis<T>::set_euler_zxy(const Vec3<T> &p_euler) {
	T c, s;

	std::tie(c,s) = sincos(p_euler.x);
	Basis<T> xmat(1, 0, 0, 0, c, -s, 0, s, c);

	std::tie(c,s) = sincos(p_euler.y);
	Basis<T> ymat(c, 0, s, 0, 1, 0, -s, 0, c);

	std::tie(c,s) = sincos(p_euler.z);
	Basis<T> zmat(c, -s, 0, s, c, 0, 0, 0, 1);
	(*this) = zmat * xmat * ymat;
}
template<arithmetic T>
Vec3<T> Basis<T>::get_euler_zyx() const {
	// Euler angles in ZYX convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cz*cy             cz*sy*sx-cx*sz        sz*sx+cz*cx*cy
	//        cy*sz             cz*cx+sz*sy*sx        cx*sz*sy-cz*sx
	//        -sy               cy*sx                 cy*cx
	Vec3<T> euler;
	T sy = (*this)[2][0];
	if (sy < (1 - CMP_EPSILON)) {
		if (sy > -(1 - CMP_EPSILON)) {
			euler.x = atan2f((*this)[2][1], (*this)[2][2]);
			euler.y = asinf(-sy);
			euler.z = atan2f((*this)[1][0], (*this)[0][0]);
		} else {
			// It's -1
			euler.x = 0;
			euler.y = PI / 2.0;
			euler.z = -atan2f((*this)[0][1], (*this)[1][1]);
		}
	} else {
		// It's 1
		euler.x = 0;
		euler.y = -PI / 2.0;
		euler.z = -atan2f((*this)[0][1], (*this)[1][1]);
	}
	return euler;
}
template<arithmetic T>
void Basis<T>::set_euler_zyx(const Vec3<T> &p_euler) {
	T c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis<T> xmat(1, 0, 0, 0, c, -s, 0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis<T> ymat(c, 0, s, 0, 1, 0, -s, 0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis<T> zmat(c, -s, 0, s, c, 0, 0, 0, 1);

	(*this) = zmat * ymat * xmat;
}
template<arithmetic T>
bool Basis<T>::is_equal_approx(const Basis<T> &other) const {
	return (*this)[0].is_equal_approx(other*(*this)[0]) && (*this)[1].is_equal_approx(other*(*this)[1]) && (*this)[2].is_equal_approx(other*(*this)[2]);
}
template<arithmetic T>
bool Basis<T>::is_equal_approx_ratio(const Basis<T> &a, const Basis<T> &b, T p_epsilon) const {
	for (size_t i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (!is_equal_approx_f(a*(*this)[i][j], b*(*this)[i][j])) {
				return false;
			}
		}
	}

	return true;
}
template<arithmetic T>
bool Basis<T>::operator==(const Basis<T> &p_matrix) const {
	for (size_t i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if ((*this)[i][j] != p_matrix*(*this)[i][j]) {
				return false;
			}
		}
	}

	return true;
}
template<arithmetic T>
bool Basis<T>::operator!=(const Basis<T> &p_matrix) const {
	return (!((*this) == p_matrix));
}
template<arithmetic T>
Quat<T> Basis<T>::get_quat() const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!is_rotation(), Quat(), "Basis<T> must be normalized in order to be casted to a Quaternion. Use get_rotation_quat() or call orthonormalized() if the Basis<T> contains linearly independent vectors.");
#endif
	/* Allow getting a quaternion from an unnormalized transform */
	Basis<T> m = (*this);
	T trace = m*(*this)[0][0] + m*(*this)[1][1] + m*(*this)[2][2];
	T temp[4];

	if (trace > 0) {
		T s = sqrtf(trace + 1);
		temp[3] = (s * 0.5);
		s = 0.5 / s;

		temp[0] = ((m*(*this)[2][1] - m*(*this)[1][2]) * s);
		temp[1] = ((m*(*this)[0][2] - m*(*this)[2][0]) * s);
		temp[2] = ((m*(*this)[1][0] - m*(*this)[0][1]) * s);
	} else {
		size_t i = m*(*this)[0][0] < m*(*this)[1][1]
				? (m*(*this)[1][1] < m*(*this)[2][2] ? 2 : 1)
				: (m*(*this)[0][0] < m*(*this)[2][2] ? 2 : 0);
		int j = (i + 1) % 3;
		int k = (i + 2) % 3;

		T s = sqrtf(m*(*this)[i][i] - m*(*this)[j][j] - m*(*this)[k][k] + 1);
		temp[i] = s * 0.5;
		s = 0.5 / s;

		temp[3] = (m*(*this)[k][j] - m*(*this)[j][k]) * s;
		temp[j] = (m*(*this)[j][i] + m*(*this)[i][j]) * s;
		temp[k] = (m*(*this)[k][i] + m*(*this)[i][k]) * s;
	}

	return Quat<T>(temp[0], temp[1], temp[2], temp[3]);
}
template<arithmetic T>
static Basis<T> get_ortho_bases(const size_t index) {
    static const Basis<T> _ortho_bases[24] = {
        Basis<T>(1, 0, 0, 0, 1, 0, 0, 0, 1),
        Basis<T>(0, -1, 0, 1, 0, 0, 0, 0, 1),
        Basis<T>(-1, 0, 0, 0, -1, 0, 0, 0, 1),
        Basis<T>(0, 1, 0, -1, 0, 0, 0, 0, 1),
        Basis<T>(1, 0, 0, 0, 0, -1, 0, 1, 0),
        Basis<T>(0, 0, 1, 1, 0, 0, 0, 1, 0),
        Basis<T>(-1, 0, 0, 0, 0, 1, 0, 1, 0),
        Basis<T>(0, 0, -1, -1, 0, 0, 0, 1, 0),
        Basis<T>(1, 0, 0, 0, -1, 0, 0, 0, -1),
        Basis<T>(0, 1, 0, 1, 0, 0, 0, 0, -1),
        Basis<T>(-1, 0, 0, 0, 1, 0, 0, 0, -1),
        Basis<T>(0, -1, 0, -1, 0, 0, 0, 0, -1),
        Basis<T>(1, 0, 0, 0, 0, 1, 0, -1, 0),
        Basis<T>(0, 0, -1, 1, 0, 0, 0, -1, 0),
        Basis<T>(-1, 0, 0, 0, 0, -1, 0, -1, 0),
        Basis<T>(0, 0, 1, -1, 0, 0, 0, -1, 0),
        Basis<T>(0, 0, 1, 0, 1, 0, -1, 0, 0),
        Basis<T>(0, -1, 0, 0, 0, 1, -1, 0, 0),
        Basis<T>(0, 0, -1, 0, -1, 0, -1, 0, 0),
        Basis<T>(0, 1, 0, 0, 0, -1, -1, 0, 0),
        Basis<T>(0, 0, 1, 0, -1, 0, 1, 0, 0),
        Basis<T>(0, 1, 0, 0, 0, 1, 1, 0, 0),
        Basis<T>(0, 0, -1, 0, 1, 0, 1, 0, 0),
        Basis<T>(0, -1, 0, 0, 0, -1, 1, 0, 0)
    };
    return _ortho_bases[index];
}

template<arithmetic T>
size_t Basis<T>::get_orthogonal_index() const {
	//could be sped up if i come up with a way
	Basis<T> orth = (*this);
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			T v = orth[i][j];
			if (v > 0.5) {
				v = 1;
			} else if (v < -0.5) {
				v = -1;
			} else {
				v = 0;
			}

			orth[i][j] = v;
		}
	}

	for (size_t i = 0; i < 24; i++) {
		if (get_ortho_bases<T>(i) == orth) {
			return i;
		}
	}

	return 0;
}

template<arithmetic T>
void Basis<T>::set_orthogonal_index(size_t p_index) {
	(*this) = get_ortho_bases<T>(p_index);
}

template<arithmetic T>
void Basis<T>::get_axis_angle(Vec3<T> &r_axis, T &r_angle) const {
	/* checking this is a bad idea, because obtaining from scaled transform is a valid use case
#ifdef MATH_CHECKS
	ERR_FAIL_COND(!is_rotation());
#endif
*/
	T angle;
	T _x;
	T _y;
	T _z; // variables for result
	T epsilon = 01; // margin to allow for rounding errors
	T epsilon2 = 0.1; // margin to distinguish between 0 and 180 degrees

	if ((fabs((*this)[1][0] - (*this)[0][1]) < epsilon) && (fabs((*this)[2][0] - (*this)[0][2]) < epsilon) && (fabs((*this)[2][1] - (*this)[1][2]) < epsilon)) {
		// singularity found
		// first check for identity matrix which must have +1 for all terms
		//  in leading diagonaland zero in other terms
		if ((fabs((*this)[1][0] + (*this)[0][1]) < epsilon2) && (fabs((*this)[2][0] + (*this)[0][2]) < epsilon2) && (fabs((*this)[2][1] + (*this)[1][2]) < epsilon2) && (fabs((*this)[0][0] + (*this)[1][1] + (*this)[2][2] - 3) < epsilon2)) {
			// this singularity is identity matrix so angle = 0
			r_axis = Vec3<T>(0, 1, 0);
			r_angle = 0;
			return;
		}
		// otherwise this singularity is angle = 180
		angle = PI;
		T xx = ((*this)[0][0] + 1) / 2;
		T yy = ((*this)[1][1] + 1) / 2;
		T zz = ((*this)[2][2] + 1) / 2;
		T xy = ((*this)[1][0] + (*this)[0][1]) / 4;
		T xz = ((*this)[2][0] + (*this)[0][2]) / 4;
		T yz = ((*this)[2][1] + (*this)[1][2]) / 4;
		if ((xx > yy) && (xx > zz)) { // (*this)[0][0] is the largest diagonal term
			if (xx < epsilon) {
				_x = 0;
				_y = SQRT12;
				_z = SQRT12;
			} else {
				_x = sqrtf(xx);
				_y = xy / _x;
				_z = xz / _x;
			}
		} else if (yy > zz) { // (*this)[1][1] is the largest diagonal term
			if (yy < epsilon) {
				_x = SQRT12;
				_y = 0;
				_z = SQRT12;
			} else {
				_y = sqrtf(yy);
				_x = xy / _y;
				_z = yz / _y;
			}
		} else { // (*this)[2][2] is the largest diagonal term so base result on this
			if (zz < epsilon) {
				_x = SQRT12;
				_y = SQRT12;
				_z = 0;
			} else {
				_z = sqrtf(zz);
				_x = xz / _z;
				_y = yz / _z;
			}
		}
		r_axis = Vec3<T>(_x, _y, _z);
		r_angle = angle;
		return;
	}
	// as we have reached here there are no singularities so we can handle normally
	T s = sqrtf(((*this)[1][2] - (*this)[2][1]) * ((*this)[1][2] - 
				(*this)[2][1]) + ((*this)[2][0] - (*this)[0][2]) * 
				((*this)[2][0] - (*this)[0][2]) + ((*this)[0][1] - 
				(*this)[1][0]) * ((*this)[0][1] - (*this)[1][0])); // s=|axis||sin(angle)|, used to normalise

	angle = acosf(((*this)[0][0] + (*this)[1][1] + (*this)[2][2] - 1) / 2);
	if (angle < 0) {
		s = -s;
	}

	_x = ((*this)[2][1] - (*this)[1][2]) / s;
	_y = ((*this)[0][2] - (*this)[2][0]) / s;
	_z = ((*this)[1][0] - (*this)[0][1]) / s;

	r_axis = Vec3<T>(_x, _y, _z);
	r_angle = angle;
}

template<arithmetic T>
void Basis<T>::set_quat(const Quat<T> &p_quat) {
	T d = p_quat.length_squared();
	T s = static_cast<T>(2) / d;
	T xs = p_quat.x * s, ys = p_quat.y * s, zs = p_quat.z * s;
	T wx = p_quat.w * xs, wy = p_quat.w * ys, wz = p_quat.w * zs;
	T xx = p_quat.x * xs, xy = p_quat.x * ys, xz = p_quat.x * zs;
	T yy = p_quat.y * ys, yz = p_quat.y * zs, zz = p_quat.z * zs;
	set(1 - (yy + zz), xy - wz, xz + wy,
			xy + wz, 1 - (xx + zz), yz - wx,
			xz - wy, yz + wx, 1 - (xx + yy));
}

template<arithmetic T>
void Basis<T>::set_axis_angle(const Vec3<T> &p_axis, T p_phi) {
// Rotation matrix from axis and angle, see https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_angle
#ifdef MATH_CHECKS
	ERR_FAIL_COND_MSG(!p_axis.is_normalized(), "The axis Vec3<T> must be normalized.");
#endif
	Vec3<T> axis_sq(p_axis.x * p_axis.x, p_axis.y * p_axis.y, p_axis.z * p_axis.z);
	T cosine = cosf(p_phi);
	(*this)[0][0] = axis_sq.x + cosine * (1 - axis_sq.x);
	(*this)[1][1] = axis_sq.y + cosine * (1 - axis_sq.y);
	(*this)[2][2] = axis_sq.z + cosine * (1 - axis_sq.z);

	T sine = sinf(p_phi);
	T _t = 1 - cosine;

	T xyzt = p_axis.x * p_axis.y * _t;
	T zyxs = p_axis.z * sine;
	(*this)[0][1] = xyzt - zyxs;
	(*this)[1][0] = xyzt + zyxs;

	xyzt = p_axis.x * p_axis.z * _t;
	zyxs = p_axis.y * sine;
	(*this)[0][2] = xyzt + zyxs;
	(*this)[2][0] = xyzt - zyxs;

	xyzt = p_axis.y * p_axis.z * _t;
	zyxs = p_axis.x * sine;
	(*this)[1][2] = xyzt - zyxs;
	(*this)[2][1] = xyzt + zyxs;
}
template<arithmetic T>
void Basis<T>::set_axis_angle_scale(const Vec3<T> &p_axis, T p_phi, const Vec3<T> &p_scale) {
	set_diagonal(p_scale);
	rotate(p_axis, p_phi);
}
template<arithmetic T>
void Basis<T>::set_euler_scale(const Vec3<T> &p_euler, const Vec3<T> &p_scale) {
	set_diagonal(p_scale);
	rotate(p_euler);
}
template<arithmetic T>
void Basis<T>::set_quat_scale(const Quat<T> &p_quat, const Vec3<T> &p_scale) {
	set_diagonal(p_scale);
	rotate(p_quat);
}
template<arithmetic T>
void Basis<T>::set_diagonal(const Vec3<T> &p_diag) {
	(*this)[0][0] = p_diag.x;
	(*this)[0][1] = 0;
	(*this)[0][2] = 0;

	(*this)[1][0] = 0;
	(*this)[1][1] = p_diag.y;
	(*this)[1][2] = 0;

	(*this)[2][0] = 0;
	(*this)[2][1] = 0;
	(*this)[2][2] = p_diag.z;
}

template<arithmetic T>
Basis<T> Basis<T>::slerp(const Basis<T> &p_to, const T p_weight) const {
	//consider scale
	Quat<T> from((*this));
	Quat<T> to(p_to);

	Basis<T> b(from.slerp(to, p_weight));
	b*(*this)[0] *= lerp((*this)[0].length(), p_to*(*this)[0].length(), p_weight);
	b*(*this)[1] *= lerp((*this)[1].length(), p_to*(*this)[1].length(), p_weight);
	b*(*this)[2] *= lerp((*this)[2].length(), p_to*(*this)[2].length(), p_weight);

	return b;
}

}


#undef SQRT12
#undef cofac
