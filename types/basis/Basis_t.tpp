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

#define SQRT12 static_cast<T>(3.464101615137754f)
#define cofac(row1, col1, row2, col2) ((*this)[row1][col1] * (*this)[row2][col2] - (*this)[row1][col2] * (*this)[row2][col1])

template<arithmetic T>
void Basis_t<T>::from_z(const Vector3_t<T> &p_z) {
	if (fabs(p_z.z) > SQRT12) {
		// choose p in y-z plane
		T a = p_z[1] * p_z[1] + p_z[2] * p_z[2];
		T k = 1 / sqrtf(a);
		(*this)[0] = Vector3_t<T>(0, -p_z[2] * k, p_z[1] * k);
		(*this)[1] = Vector3_t<T>(a * k, -p_z[0] * (*this)[0][2], p_z[0] * (*this)[0][1]);
	} else {
		// choose p in x-y plane
		T a = p_z.x * p_z.x + p_z.y * p_z.y;
		T k = 1 / sqrtf(a);
		(*this)[0] = Vector3_t<T>(-p_z.y * k, p_z.x * k, 0);
		(*this)[1] = Vector3_t<T>(-p_z.z * (*this)[0].y, p_z.z * (*this)[0].x, a * k);
	}
	(*this)[2] = p_z;
}
template<arithmetic T>
void Basis_t<T>::invert() {
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
void Basis_t<T>::orthonormalize() {
	// Gram-Schmidt Process

	Vector3_t<T> _x = get_axis(0);
	Vector3_t<T> _y = get_axis(1);
	Vector3_t<T> _z = get_axis(2);

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
Basis_t<T> Basis_t<T>::orthonormalized() const {
	Basis_t<T> c = (*this);
	c.orthonormalize();
	return c;
}
template<arithmetic T>
bool Basis_t<T>::is_orthogonal() const {
	Basis_t<T> identity;
	Basis_t<T> m = ((*this)) * transposed();

	return m.is_equal_approx(identity);
}
template<arithmetic T>
bool Basis_t<T>::is_diagonal() const {
	return (
			is_zero_approx((*this)[0][1]) && is_zero_approx((*this)[0][2]) &&
			is_zero_approx((*this)[1][0]) && is_zero_approx((*this)[1][2]) &&
			is_zero_approx((*this)[2][0]) && is_zero_approx((*this)[2][1]));
}
template<arithmetic T>
bool Basis_t<T>::is_rotation() const {
	return is_equal_approx_f(determinant(), 1) && is_orthogonal();
}
template<arithmetic T>
bool Basis_t<T>::is_symmetric() const {
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
Basis_t<T> Basis_t<T>::diagonalize() {
//NOTE: only implemented for symmetric matrices
//with the Jacobi iterative method method

	const size_t ite_max = 1024;

	T off_matrix_norm_2 = (*this)[0][1] * (*this)[0][1] + (*this)[0][2] * (*this)[0][2] + (*this)[1][2] * (*this)[1][2];

	size_t ite = 0;
	Basis_t<T> acc_rot;
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
		Basis_t<T> rot;
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
Basis_t<T> Basis_t<T>::inverse() const {
	Basis_t<T> inv = (*this);
	inv.invert();
	return inv;
}
template<arithmetic T>
void Basis_t<T>::transpose() {
	std::swap((*this)[0][1], (*this)[1][0]);
	std::swap((*this)[0][2], (*this)[2][0]);
	std::swap((*this)[1][2], (*this)[2][1]);
}
template<arithmetic T>
Basis_t<T> Basis_t<T>::transposed() const {
	Basis_t<T> tr = (*this);
	tr.transpose();
	return tr;
}

// Multiplies the matrix from left by the scaling matrix: M -> S.M
// See the comment for Basis_t<T>::rotated for further explanation.
template<arithmetic T>
void Basis_t<T>::scale(const Vector3_t<T> &p_scale) {
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
Basis_t<T> Basis_t<T>::scaled(const Vector3_t<T> &p_scale) const {
	Basis_t<T> m = (*this);
	m.scale(p_scale);
	return m;
}
template<arithmetic T>
void Basis_t<T>::scale_local(const Vector3_t<T> &p_scale) {
	// performs a scaling in object-local coordinate system:
	// M -> (M.S.Minv).M = M.S.
	(*this) = scaled_local(p_scale);
}
template<arithmetic T>
Basis_t<T> Basis_t<T>::scaled_local(const Vector3_t<T> &p_scale) const {
	Basis_t<T> b;
	b.set_diagonal(p_scale);

	return ((*this)) * b;
}
template<arithmetic T>
Vector3_t<T> Basis_t<T>::get_scale_abs() const {
	return Vector3_t<T>(
			Vector3_t<T>((*this)[0][0], (*this)[1][0], (*this)[2][0]).length(),
			Vector3_t<T>((*this)[0][1], (*this)[1][1], (*this)[2][1]).length(),
			Vector3_t<T>((*this)[0][2], (*this)[1][2], (*this)[2][2]).length());
}
template<arithmetic T>
Vector3_t<T> Basis_t<T>::get_scale_local() const {
	T det_sign = SGN(determinant());
	return Vector3_t<T>((*this)[0].length(), (*this)[1].length(), (*this)[2].length()) * det_sign;
}

// get_scale works with get_rotation, use get_scale_abs if you need to enforce positive signature.
template<arithmetic T>
Vector3_t<T> Basis_t<T>::get_scale() const {
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
	// as a part of Basis_t<T>. However, if we go that path, we need to disable direct (write) access to the
	// matrix (*this).
	//
	// The rotation part of this decomposition is returned by get_rotation* functions.
	T det_sign = SGN(determinant());
	return get_scale_abs() * det_sign;
}

// Decomposes a Basis_t<T> into a rotation-reflection matrix (an element of the group O(3)) and a positive scaling matrix as B = O.S.
// Returns the rotation-reflection matrix via reference argument, and scaling information is returned as a Vector3_t<T>.
// This (internal) function is too specific and named too ugly to expose to users, and probably there's no need to do so.
template<arithmetic T>
Vector3_t<T> Basis_t<T>::rotref_posscale_decomposition(Basis_t<T> &rotref) const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V(determinant() == 0, Vector3_t<T>());

	Basis_t<T> m = transposed() * ((*this));
	ERR_FAIL_COND_V(!m.is_diagonal(), Vector3_t<T>());
#endif
	Vector3_t<T> scale = get_scale();
	Basis_t<T> inv_scale = Basis_t<T>().scaled(scale.inverse()); // this will also absorb the sign of scale
	rotref = ((*this)) * inv_scale;

#ifdef MATH_CHECKS
	ERR_FAIL_COND_V(!rotref.is_orthogonal(), Vector3_t<T>());
#endif
	return scale.abs();
}



template<arithmetic T>
void Basis_t<T>::rotate(const Vector3_t<T> &p_axis, T p_phi) {
	(*this) = rotated(p_axis, p_phi);
}
template<arithmetic T>
void Basis_t<T>::rotate_local(const Vector3_t<T> &p_axis, T p_phi) {
	// performs a rotation in object-local coordinate system:
	// M -> (M.R.Minv).M = M.R.
	(*this) = rotated_local(p_axis, p_phi);
}
template<arithmetic T>
Basis_t<T> Basis_t<T>::rotated_local(const Vector3_t<T> &p_axis, T p_phi) const {
	return ((*this)) * Basis_t<T>(p_axis, p_phi);
}
template<arithmetic T>
Basis_t<T> Basis_t<T>::rotated(const Vector3_t<T> &p_euler) const {
	return Basis_t<T>(p_euler) * ((*this));
}
template<arithmetic T>
void Basis_t<T>::rotate(const Vector3_t<T> &p_euler) {
	(*this) = rotated(p_euler);
}
template<arithmetic T>
Basis_t<T> Basis_t<T>::rotated(const Quat_t<T> &p_quat) const {
	return Basis_t<T>(p_quat) * ((*this));
}
template<arithmetic T>
void Basis_t<T>::rotate(const Quat_t<T> &p_quat) {
	(*this) = rotated(p_quat);
}
template<arithmetic T>
Vector3_t<T> Basis_t<T>::get_rotation_euler() const {
	// Assumes that the matrix can be decomposed into a proper rotation and scaling matrix as M = R.S,
	// and returns the Euler angles corresponding to the rotation part, complementing get_scale().
	// See the comment in get_scale() for further information.
	Basis_t<T> m = orthonormalized();
	T det = m.determinant();
	if (det < 0) {
		// Ensure that the determinant is 1, such that result is a proper rotation matrix which can be represented by Euler angles.
		m.scale(Vector3_t<T>(-1, -1, -1));
	}

	return m.get_euler();
}
template<arithmetic T>
Quat_t<T> Basis_t<T>::get_rotation_quat() const {
	// Assumes that the matrix can be decomposed into a proper rotation and scaling matrix as M = R.S,
	// and returns the Euler angles corresponding to the rotation part, complementing get_scale().
	// See the comment in get_scale() for further information.
	Basis_t<T> m = orthonormalized();
	T det = m.determinant();
	if (det < 0) {
		// Ensure that the determinant is 1, such that result is a proper rotation matrix which can be represented by Euler angles.
		m.scale(Vector3_t<T>(-1, -1, -1));
	}

	return m.get_quat();
}
template<arithmetic T>
void Basis_t<T>::get_rotation_axis_angle(Vector3_t<T> &p_axis, T &p_angle) const {
	// Assumes that the matrix can be decomposed into a proper rotation and scaling matrix as M = R.S,
	// and returns the Euler angles corresponding to the rotation part, complementing get_scale().
	// See the comment in get_scale() for further information.
	Basis_t<T> m = orthonormalized();
	T det = m.determinant();
	if (det < 0) {
		// Ensure that the determinant is 1, such that result is a proper rotation matrix which can be represented by Euler angles.
		m.scale(Vector3_t<T>(-1, -1, -1));
	}

	m.get_axis_angle(p_axis, p_angle);
}
template<arithmetic T>
void Basis_t<T>::get_rotation_axis_angle_local(Vector3_t<T> &p_axis, T &p_angle) const {
	// Assumes that the matrix can be decomposed into a proper rotation and scaling matrix as M = R.S,
	// and returns the Euler angles corresponding to the rotation part, complementing get_scale().
	// See the comment in get_scale() for further information.
	Basis_t<T> m = transposed();
	m.orthonormalize();
	T det = m.determinant();
	if (det < 0) {
		// Ensure that the determinant is 1, such that result is a proper rotation matrix which can be represented by Euler angles.
		m.scale(Vector3_t<T>(-1, -1, -1));
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
Vector3_t<T> Basis_t<T>::get_euler_xyz() const {
	// Euler angles in XYZ convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cy*cz          -cy*sz           sy
	//        cz*sx*sy+cx*sz  cx*cz-sx*sy*sz -cy*sx
	//       -cx*cz*sy+sx*sz  cz*sx+cx*sy*sz  cx*cy

	Vector3_t<T> euler;
	T sy = (*this)[0][2];
	if (sy < (1 - CMP_EPSILON)) {
		if (sy > -(1 - CMP_EPSILON)) {
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
			euler.y = -PI / 2.0;
			euler.z = 0;
		}
	} else {
		euler.x = atan2f((*this)[2][1], (*this)[1][1]);
		euler.y = PI / 2.0;
		euler.z = 0;
	}
	return euler;
}

// set_euler_xyz expects a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// The current implementation uses XYZ convention (Z is the first rotation).
template<arithmetic T>
void Basis_t<T>::set_euler_xyz(const Vector3_t<T> &p_euler) {
	T c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis_t<T> xmat(1, 0, 0, 0, c, -s, 0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis_t<T> ymat(c, 0, s, 0, 1, 0, -s, 0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis_t<T> zmat(c, -s, 0, s, c, 0, 0, 0, 1);

	//optimizer will optimize away all this anyway
	(*this) = xmat * (ymat * zmat);
}
template<arithmetic T>
Vector3_t<T> Basis_t<T>::get_euler_xzy() const {
	// Euler angles in XZY convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cz*cy             -sz             cz*sy
	//        sx*sy+cx*cy*sz    cx*cz           cx*sz*sy-cy*sx
	//        cy*sx*sz          cz*sx           cx*cy+sx*sz*sy

	Vector3_t<T> euler;
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
void Basis_t<T>::set_euler_xzy(const Vector3_t<T> &p_euler) {
	T c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis_t<T> xmat(1, 0, 0, 0, c, -s, 0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis_t<T> ymat(c, 0, s, 0, 1, 0, -s, 0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis_t<T> zmat(c, -s, 0, s, c, 0, 0, 0, 1);

	(*this) = xmat * zmat * ymat;
}
template<arithmetic T>
Vector3_t<T> Basis_t<T>::get_euler_yzx() const {
	// Euler angles in YZX convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cy*cz             sy*sx-cy*cx*sz     cx*sy+cy*sz*sx
	//        sz                cz*cx              -cz*sx
	//        -cz*sy            cy*sx+cx*sy*sz     cy*cx-sy*sz*sx

	Vector3_t<T> euler;
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
void Basis_t<T>::set_euler_yzx(const Vector3_t<T> &p_euler) {
	T c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis_t<T> xmat(1, 0, 0, 0, c, -s, 0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis_t<T> ymat(c, 0, s, 0, 1, 0, -s, 0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis_t<T> zmat(c, -s, 0, s, c, 0, 0, 0, 1);

	(*this) = ymat * zmat * xmat;
}

// get_euler_yxz returns a vector containing the Euler angles in the YXZ convention,
// as in first-Z, then-X, last-Y. The angles for X, Y, and Z rotations are returned
// as the x, y, and z components of a Vector3_t<T> respectively.
template<arithmetic T>
Vector3_t<T> Basis_t<T>::get_euler_yxz() const {
	// Euler angles in YXZ convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cy*cz+sy*sx*sz    cz*sy*sx-cy*sz        cx*sy
	//        cx*sz             cx*cz                 -sx
	//        cy*sx*sz-cz*sy    cy*cz*sx+sy*sz        cy*cx

	Vector3_t<T> euler;

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
void Basis_t<T>::set_euler_yxz(const Vector3_t<T> &p_euler) {
	T c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis_t<T> xmat(1, 0, 0, 0, c, -s, 0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis_t<T> ymat(c, 0, s, 0, 1, 0, -s, 0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis_t<T> zmat(c, -s, 0, s, c, 0, 0, 0, 1);

	//optimizer will optimize away all this anyway
	(*this) = ymat * xmat * zmat;
}
template<arithmetic T>
Vector3_t<T> Basis_t<T>::get_euler_zxy() const {
	// Euler angles in ZXY convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cz*cy-sz*sx*sy    -cx*sz                cz*sy+cy*sz*sx
	//        cy*sz+cz*sx*sy    cz*cx                 sz*sy-cz*cy*sx
	//        -cx*sy            sx                    cx*cy
	Vector3_t<T> euler;
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
void Basis_t<T>::set_euler_zxy(const Vector3_t<T> &p_euler) {
	T c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis_t<T> xmat(1, 0, 0, 0, c, -s, 0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis_t<T> ymat(c, 0, s, 0, 1, 0, -s, 0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis_t<T> zmat(c, -s, 0, s, c, 0, 0, 0, 1);

	(*this) = zmat * xmat * ymat;
}
template<arithmetic T>
Vector3_t<T> Basis_t<T>::get_euler_zyx() const {
	// Euler angles in ZYX convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cz*cy             cz*sy*sx-cx*sz        sz*sx+cz*cx*cy
	//        cy*sz             cz*cx+sz*sy*sx        cx*sz*sy-cz*sx
	//        -sy               cy*sx                 cy*cx
	Vector3_t<T> euler;
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
void Basis_t<T>::set_euler_zyx(const Vector3_t<T> &p_euler) {
	T c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis_t<T> xmat(1, 0, 0, 0, c, -s, 0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis_t<T> ymat(c, 0, s, 0, 1, 0, -s, 0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis_t<T> zmat(c, -s, 0, s, c, 0, 0, 0, 1);

	(*this) = zmat * ymat * xmat;
}
template<arithmetic T>
bool Basis_t<T>::is_equal_approx(const Basis_t<T> &other) const {
	return (*this)[0].is_equal_approx(other*(*this)[0]) && (*this)[1].is_equal_approx(other*(*this)[1]) && (*this)[2].is_equal_approx(other*(*this)[2]);
}
template<arithmetic T>
bool Basis_t<T>::is_equal_approx_ratio(const Basis_t<T> &a, const Basis_t<T> &b, T p_epsilon) const {
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
bool Basis_t<T>::operator==(const Basis_t<T> &p_matrix) const {
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
bool Basis_t<T>::operator!=(const Basis_t<T> &p_matrix) const {
	return (!((*this) == p_matrix));
}
template<arithmetic T>
Quat_t<T> Basis_t<T>::get_quat() const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!is_rotation(), Quat(), "Basis_t<T> must be normalized in order to be casted to a Quaternion. Use get_rotation_quat() or call orthonormalized() if the Basis_t<T> contains linearly independent vectors.");
#endif
	/* Allow getting a quaternion from an unnormalized transform */
	Basis_t<T> m = (*this);
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

	return Quat_t<T>(temp[0], temp[1], temp[2], temp[3]);
}
template<arithmetic T>
static Basis_t<T> get_ortho_bases(const size_t index) {
    static const Basis_t<T> _ortho_bases[24] = {
        Basis_t<T>(1, 0, 0, 0, 1, 0, 0, 0, 1),
        Basis_t<T>(0, -1, 0, 1, 0, 0, 0, 0, 1),
        Basis_t<T>(-1, 0, 0, 0, -1, 0, 0, 0, 1),
        Basis_t<T>(0, 1, 0, -1, 0, 0, 0, 0, 1),
        Basis_t<T>(1, 0, 0, 0, 0, -1, 0, 1, 0),
        Basis_t<T>(0, 0, 1, 1, 0, 0, 0, 1, 0),
        Basis_t<T>(-1, 0, 0, 0, 0, 1, 0, 1, 0),
        Basis_t<T>(0, 0, -1, -1, 0, 0, 0, 1, 0),
        Basis_t<T>(1, 0, 0, 0, -1, 0, 0, 0, -1),
        Basis_t<T>(0, 1, 0, 1, 0, 0, 0, 0, -1),
        Basis_t<T>(-1, 0, 0, 0, 1, 0, 0, 0, -1),
        Basis_t<T>(0, -1, 0, -1, 0, 0, 0, 0, -1),
        Basis_t<T>(1, 0, 0, 0, 0, 1, 0, -1, 0),
        Basis_t<T>(0, 0, -1, 1, 0, 0, 0, -1, 0),
        Basis_t<T>(-1, 0, 0, 0, 0, -1, 0, -1, 0),
        Basis_t<T>(0, 0, 1, -1, 0, 0, 0, -1, 0),
        Basis_t<T>(0, 0, 1, 0, 1, 0, -1, 0, 0),
        Basis_t<T>(0, -1, 0, 0, 0, 1, -1, 0, 0),
        Basis_t<T>(0, 0, -1, 0, -1, 0, -1, 0, 0),
        Basis_t<T>(0, 1, 0, 0, 0, -1, -1, 0, 0),
        Basis_t<T>(0, 0, 1, 0, -1, 0, 1, 0, 0),
        Basis_t<T>(0, 1, 0, 0, 0, 1, 1, 0, 0),
        Basis_t<T>(0, 0, -1, 0, 1, 0, 1, 0, 0),
        Basis_t<T>(0, -1, 0, 0, 0, -1, 1, 0, 0)
    };
    return _ortho_bases[index];
}

template<arithmetic T>
size_t Basis_t<T>::get_orthogonal_index() const {
	//could be sped up if i come up with a way
	Basis_t<T> orth = (*this);
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
void Basis_t<T>::set_orthogonal_index(size_t p_index) {
	(*this) = get_ortho_bases<T>(p_index);
}

template<arithmetic T>
void Basis_t<T>::get_axis_angle(Vector3_t<T> &r_axis, T &r_angle) const {
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
			r_axis = Vector3_t<T>(0, 1, 0);
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
		r_axis = Vector3_t<T>(_x, _y, _z);
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

	r_axis = Vector3_t<T>(_x, _y, _z);
	r_angle = angle;
}

template<arithmetic T>
void Basis_t<T>::set_quat(const Quat_t<T> &p_quat) {
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
void Basis_t<T>::set_axis_angle(const Vector3_t<T> &p_axis, T p_phi) {
// Rotation matrix from axis and angle, see https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_angle
#ifdef MATH_CHECKS
	ERR_FAIL_COND_MSG(!p_axis.is_normalized(), "The axis Vector3_t<T> must be normalized.");
#endif
	Vector3_t<T> axis_sq(p_axis.x * p_axis.x, p_axis.y * p_axis.y, p_axis.z * p_axis.z);
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
void Basis_t<T>::set_axis_angle_scale(const Vector3_t<T> &p_axis, T p_phi, const Vector3_t<T> &p_scale) {
	set_diagonal(p_scale);
	rotate(p_axis, p_phi);
}
template<arithmetic T>
void Basis_t<T>::set_euler_scale(const Vector3_t<T> &p_euler, const Vector3_t<T> &p_scale) {
	set_diagonal(p_scale);
	rotate(p_euler);
}
template<arithmetic T>
void Basis_t<T>::set_quat_scale(const Quat_t<T> &p_quat, const Vector3_t<T> &p_scale) {
	set_diagonal(p_scale);
	rotate(p_quat);
}
template<arithmetic T>
void Basis_t<T>::set_diagonal(const Vector3_t<T> &p_diag) {
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
Basis_t<T> Basis_t<T>::slerp(const Basis_t<T> &p_to, const T p_weight) const {
	//consider scale
	Quat_t<T> from((*this));
	Quat_t<T> to(p_to);

	Basis_t<T> b(from.slerp(to, p_weight));
	b*(*this)[0] *= lerp((*this)[0].length(), p_to*(*this)[0].length(), p_weight);
	b*(*this)[1] *= lerp((*this)[1].length(), p_to*(*this)[1].length(), p_weight);
	b*(*this)[2] *= lerp((*this)[2].length(), p_to*(*this)[2].length(), p_weight);

	return b;
}



#undef SQRT12
#undef cofac
