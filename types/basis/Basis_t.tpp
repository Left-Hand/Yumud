#define SQRT12 3.464101615137754f

#define cofac(row1, col1, row2, col2) (elements[row1][col1] * elements[row2][col2] - elements[row1][col2] * elements[row2][col1])

template<typename real>
void Basis_t<real>::from_z(const Vector3_t<real> &p_z) {
	if (fabs(p_z.z) > SQRT12) {
		// choose p in y-z plane
		real a = p_z[1] * p_z[1] + p_z[2] * p_z[2];
		real k = 1.0 / sqrtf(a);
		elements[0] = Vector3_t<real>(0, -p_z[2] * k, p_z[1] * k);
		elements[1] = Vector3_t<real>(a * k, -p_z[0] * elements[0][2], p_z[0] * elements[0][1]);
	} else {
		// choose p in x-y plane
		real a = p_z.x * p_z.x + p_z.y * p_z.y;
		real k = 1.0 / sqrtf(a);
		elements[0] = Vector3_t<real>(-p_z.y * k, p_z.x * k, 0);
		elements[1] = Vector3_t<real>(-p_z.z * elements[0].y, p_z.z * elements[0].x, a * k);
	}
	elements[2] = p_z;
}
template<typename real>
void Basis_t<real>::invert() {
	real co[3] = {
		cofac(1, 1, 2, 2), cofac(1, 2, 2, 0), cofac(1, 0, 2, 1)
	};
	real det = elements[0][0] * co[0] +
			elements[0][1] * co[1] +
			elements[0][2] * co[2];
	real s = 1.0 / det;

	set(co[0] * s, cofac(0, 2, 2, 1) * s, cofac(0, 1, 1, 2) * s,
			co[1] * s, cofac(0, 0, 2, 2) * s, cofac(0, 2, 1, 0) * s,
			co[2] * s, cofac(0, 1, 2, 0) * s, cofac(0, 0, 1, 1) * s);
}
template<typename real>
void Basis_t<real>::orthonormalize() {
	// Gram-Schmidt Process

	Vector3_t<real> x = get_axis(0);
	Vector3_t<real> y = get_axis(1);
	Vector3_t<real> z = get_axis(2);

	x.normalize();
	y = (y - x * (x.dot(y)));
	y.normalize();
	z = (z - x * (x.dot(z)) - y * (y.dot(z)));
	z.normalize();

	set_axis(0, x);
	set_axis(1, y);
	set_axis(2, z);
}
template<typename real>
Basis_t<real> Basis_t<real>::orthonormalized() const {
	Basis_t<real> c = *this;
	c.orthonormalize();
	return c;
}
template<typename real>
bool Basis_t<real>::is_orthogonal() const {
	Basis_t<real> identity;
	Basis_t<real> m = (*this) * transposed();

	return m.is_equal_approx(identity);
}
template<typename real>
bool Basis_t<real>::is_diagonal() const {
	return (
			is_zero_approx(elements[0][1]) && is_zero_approx(elements[0][2]) &&
			is_zero_approx(elements[1][0]) && is_zero_approx(elements[1][2]) &&
			is_zero_approx(elements[2][0]) && is_zero_approx(elements[2][1]));
}
template<typename real>
bool Basis_t<real>::is_rotation() const {
	return is_equal_approx_f(determinant(), 1.0f) && is_orthogonal();
}
template<typename real>
bool Basis_t<real>::is_symmetric() const {
	if (!is_equal_approx_f(elements[0][1], elements[1][0])) {
		return false;
	}
	if (!is_equal_approx_f(elements[0][2], elements[2][0])) {
		return false;
	}
	if (!is_equal_approx_f(elements[1][2], elements[2][1])) {
		return false;
	}

	return true;
}
template<typename real>
Basis_t<real> Basis_t<real>::diagonalize() {
//NOTE: only implemented for symmetric matrices
//with the Jacobi iterative method method

	const int ite_max = 1024;

	real off_matrix_norm_2 = elements[0][1] * elements[0][1] + elements[0][2] * elements[0][2] + elements[1][2] * elements[1][2];

	int ite = 0;
	Basis_t<real> acc_rot;
	while (off_matrix_norm_2 > CMP_EPSILON && ite++ < ite_max) {
		real el01_2 = elements[0][1] * elements[0][1];
		real el02_2 = elements[0][2] * elements[0][2];
		real el12_2 = elements[1][2] * elements[1][2];
		// Find the pivot element
		int i, j;
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
		real angle;
		if (is_equal_approx_f(elements[j][j], elements[i][i])) {
			angle = PI / 4;
		} else {
			angle = 0.5 * atanf(2 * elements[i][j] / (elements[j][j] - elements[i][i]));
		}

		// Compute the rotation matrix
		Basis_t<real> rot;
		rot.elements[i][i] = rot.elements[j][j] = cosf(angle);
		rot.elements[i][j] = -(rot.elements[j][i] = sinf(angle));

		// Update the off matrix norm
		off_matrix_norm_2 -= elements[i][j] * elements[i][j];

		// Apply the rotation
		*this = rot * *this * rot.transposed();
		acc_rot = rot * acc_rot;
	}

	return acc_rot;
}
template<typename real>
Basis_t<real> Basis_t<real>::inverse() const {
	Basis_t<real> inv = *this;
	inv.invert();
	return inv;
}
template<typename real>
void Basis_t<real>::transpose() {
	SWAP(elements[0][1], elements[1][0]);
	SWAP(elements[0][2], elements[2][0]);
	SWAP(elements[1][2], elements[2][1]);
}
template<typename real>
Basis_t<real> Basis_t<real>::transposed() const {
	Basis_t<real> tr = *this;
	tr.transpose();
	return tr;
}

// Multiplies the matrix from left by the scaling matrix: M -> S.M
// See the comment for Basis_t<real>::rotated for further explanation.
template<typename real>
void Basis_t<real>::scale(const Vector3_t<real> &p_scale) {
	elements[0][0] *= p_scale.x;
	elements[0][1] *= p_scale.x;
	elements[0][2] *= p_scale.x;
	elements[1][0] *= p_scale.y;
	elements[1][1] *= p_scale.y;
	elements[1][2] *= p_scale.y;
	elements[2][0] *= p_scale.z;
	elements[2][1] *= p_scale.z;
	elements[2][2] *= p_scale.z;
}
template<typename real>
Basis_t<real> Basis_t<real>::scaled(const Vector3_t<real> &p_scale) const {
	Basis_t<real> m = *this;
	m.scale(p_scale);
	return m;
}
template<typename real>
void Basis_t<real>::scale_local(const Vector3_t<real> &p_scale) {
	// performs a scaling in object-local coordinate system:
	// M -> (M.S.Minv).M = M.S.
	*this = scaled_local(p_scale);
}
template<typename real>
Basis_t<real> Basis_t<real>::scaled_local(const Vector3_t<real> &p_scale) const {
	Basis_t<real> b;
	b.set_diagonal(p_scale);

	return (*this) * b;
}
template<typename real>
Vector3_t<real> Basis_t<real>::get_scale_abs() const {
	return Vector3_t<real>(
			Vector3_t<real>(elements[0][0], elements[1][0], elements[2][0]).length(),
			Vector3_t<real>(elements[0][1], elements[1][1], elements[2][1]).length(),
			Vector3_t<real>(elements[0][2], elements[1][2], elements[2][2]).length());
}
template<typename real>
Vector3_t<real> Basis_t<real>::get_scale_local() const {
	real det_sign = SGN(determinant());
	return Vector3_t<real>(elements[0].length(), elements[1].length(), elements[2].length()) * det_sign;
}

// get_scale works with get_rotation, use get_scale_abs if you need to enforce positive signature.
template<typename real>
Vector3_t<real> Basis_t<real>::get_scale() const {
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
	// as a part of Basis_t<real>. However, if we go that path, we need to disable direct (write) access to the
	// matrix elements.
	//
	// The rotation part of this decomposition is returned by get_rotation* functions.
	real det_sign = SGN(determinant());
	return get_scale_abs() * det_sign;
}

// Decomposes a Basis_t<real> into a rotation-reflection matrix (an element of the group O(3)) and a positive scaling matrix as B = O.S.
// Returns the rotation-reflection matrix via reference argument, and scaling information is returned as a Vector3_t<real>.
// This (internal) function is too specific and named too ugly to expose to users, and probably there's no need to do so.
template<typename real>
Vector3_t<real> Basis_t<real>::rotref_posscale_decomposition(Basis_t<real> &rotref) const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V(determinant() == 0, Vector3_t<real>());

	Basis_t<real> m = transposed() * (*this);
	ERR_FAIL_COND_V(!m.is_diagonal(), Vector3_t<real>());
#endif
	Vector3_t<real> scale = get_scale();
	Basis_t<real> inv_scale = Basis_t<real>().scaled(scale.inverse()); // this will also absorb the sign of scale
	rotref = (*this) * inv_scale;

#ifdef MATH_CHECKS
	ERR_FAIL_COND_V(!rotref.is_orthogonal(), Vector3_t<real>());
#endif
	return scale.abs();
}

// Multiplies the matrix from left by the rotation matrix: M -> R.M
// Note that this does *not* rotate the matrix itself.
//
// The main use of Basis_t<real> is as Transform.Basis_t<real>, which is used a the transformation matrix
// of 3D object. Rotate here refers to rotation of the object (which is R * (*this)),
// not the matrix itself (which is R * (*this) * R.transposed()).
template<typename real>
Basis_t<real> Basis_t<real>::rotated(const Vector3_t<real> &p_axis, real p_phi) const {
	return Basis_t<real>(p_axis, p_phi) * (*this);
}
template<typename real>
void Basis_t<real>::rotate(const Vector3_t<real> &p_axis, real p_phi) {
	*this = rotated(p_axis, p_phi);
}
template<typename real>
void Basis_t<real>::rotate_local(const Vector3_t<real> &p_axis, real p_phi) {
	// performs a rotation in object-local coordinate system:
	// M -> (M.R.Minv).M = M.R.
	*this = rotated_local(p_axis, p_phi);
}
template<typename real>
Basis_t<real> Basis_t<real>::rotated_local(const Vector3_t<real> &p_axis, real p_phi) const {
	return (*this) * Basis_t<real>(p_axis, p_phi);
}
template<typename real>
Basis_t<real> Basis_t<real>::rotated(const Vector3_t<real> &p_euler) const {
	return Basis_t<real>(p_euler) * (*this);
}
template<typename real>
void Basis_t<real>::rotate(const Vector3_t<real> &p_euler) {
	*this = rotated(p_euler);
}
// template<typename real>
// Basis_t<real> Basis_t<real>::rotated(const Quat &p_quat) const {
// 	return Basis_t<real>(p_quat) * (*this);
// }
// template<typename real>
// void Basis_t<real>::rotate(const Quat &p_quat) {
// 	*this = rotated(p_quat);
// }
template<typename real>
Vector3_t<real> Basis_t<real>::get_rotation_euler() const {
	// Assumes that the matrix can be decomposed into a proper rotation and scaling matrix as M = R.S,
	// and returns the Euler angles corresponding to the rotation part, complementing get_scale().
	// See the comment in get_scale() for further information.
	Basis_t<real> m = orthonormalized();
	real det = m.determinant();
	if (det < 0) {
		// Ensure that the determinant is 1, such that result is a proper rotation matrix which can be represented by Euler angles.
		m.scale(Vector3_t<real>(-1, -1, -1));
	}

	return m.get_euler();
}
// template<typename real>
// Quat Basis_t<real>::get_rotation_quat() const {
// 	// Assumes that the matrix can be decomposed into a proper rotation and scaling matrix as M = R.S,
// 	// and returns the Euler angles corresponding to the rotation part, complementing get_scale().
// 	// See the comment in get_scale() for further information.
// 	Basis_t<real> m = orthonormalized();
// 	real det = m.determinant();
// 	if (det < 0) {
// 		// Ensure that the determinant is 1, such that result is a proper rotation matrix which can be represented by Euler angles.
// 		m.scale(Vector3_t<real>(-1, -1, -1));
// 	}

// 	return m.get_quat();
// }
template<typename real>
void Basis_t<real>::get_rotation_axis_angle(Vector3_t<real> &p_axis, real &p_angle) const {
	// Assumes that the matrix can be decomposed into a proper rotation and scaling matrix as M = R.S,
	// and returns the Euler angles corresponding to the rotation part, complementing get_scale().
	// See the comment in get_scale() for further information.
	Basis_t<real> m = orthonormalized();
	real det = m.determinant();
	if (det < 0) {
		// Ensure that the determinant is 1, such that result is a proper rotation matrix which can be represented by Euler angles.
		m.scale(Vector3_t<real>(-1, -1, -1));
	}

	m.get_axis_angle(p_axis, p_angle);
}
template<typename real>
void Basis_t<real>::get_rotation_axis_angle_local(Vector3_t<real> &p_axis, real &p_angle) const {
	// Assumes that the matrix can be decomposed into a proper rotation and scaling matrix as M = R.S,
	// and returns the Euler angles corresponding to the rotation part, complementing get_scale().
	// See the comment in get_scale() for further information.
	Basis_t<real> m = transposed();
	m.orthonormalize();
	real det = m.determinant();
	if (det < 0) {
		// Ensure that the determinant is 1, such that result is a proper rotation matrix which can be represented by Euler angles.
		m.scale(Vector3_t<real>(-1, -1, -1));
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
template<typename real>
Vector3_t<real> Basis_t<real>::get_euler_xyz() const {
	// Euler angles in XYZ convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cy*cz          -cy*sz           sy
	//        cz*sx*sy+cx*sz  cx*cz-sx*sy*sz -cy*sx
	//       -cx*cz*sy+sx*sz  cz*sx+cx*sy*sz  cx*cy

	Vector3_t<real> euler;
	real sy = elements[0][2];
	if (sy < (1.0 - CMP_EPSILON)) {
		if (sy > -(1.0 - CMP_EPSILON)) {
			// is this a pure Y rotation?
			if (elements[1][0] == 0.0 && elements[0][1] == 0.0 && elements[1][2] == 0 && elements[2][1] == 0 && elements[1][1] == 1) {
				// return the simplest form (human friendlier in editor and scripts)
				euler.x = 0;
				euler.y = atan2(elements[0][2], elements[0][0]);
				euler.z = 0;
			} else {
				euler.x = atan2f(-elements[1][2], elements[2][2]);
				euler.y = asinf(sy);
				euler.z = atan2f(-elements[0][1], elements[0][0]);
			}
		} else {
			euler.x = atan2f(elements[2][1], elements[1][1]);
			euler.y = -PI / 2.0;
			euler.z = 0.0;
		}
	} else {
		euler.x = atan2f(elements[2][1], elements[1][1]);
		euler.y = PI / 2.0;
		euler.z = 0.0;
	}
	return euler;
}

// set_euler_xyz expects a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// The current implementation uses XYZ convention (Z is the first rotation).
template<typename real>
void Basis_t<real>::set_euler_xyz(const Vector3_t<real> &p_euler) {
	real c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis_t<real> xmat(1.0, 0.0, 0.0, 0.0, c, -s, 0.0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis_t<real> ymat(c, 0.0, s, 0.0, 1.0, 0.0, -s, 0.0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis_t<real> zmat(c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0);

	//optimizer will optimize away all this anyway
	*this = xmat * (ymat * zmat);
}
template<typename real>
Vector3_t<real> Basis_t<real>::get_euler_xzy() const {
	// Euler angles in XZY convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cz*cy             -sz             cz*sy
	//        sx*sy+cx*cy*sz    cx*cz           cx*sz*sy-cy*sx
	//        cy*sx*sz          cz*sx           cx*cy+sx*sz*sy

	Vector3_t<real> euler;
	real sz = elements[0][1];
	if (sz < (1.0 - CMP_EPSILON)) {
		if (sz > -(1.0 - CMP_EPSILON)) {
			euler.x = atan2f(elements[2][1], elements[1][1]);
			euler.y = atan2f(elements[0][2], elements[0][0]);
			euler.z = asinf(-sz);
		} else {
			// It's -1
			euler.x = -atan2f(elements[1][2], elements[2][2]);
			euler.y = 0.0;
			euler.z = PI / 2.0;
		}
	} else {
		// It's 1
		euler.x = -atan2f(elements[1][2], elements[2][2]);
		euler.y = 0.0;
		euler.z = -PI / 2.0;
	}
	return euler;
}
template<typename real>
void Basis_t<real>::set_euler_xzy(const Vector3_t<real> &p_euler) {
	real c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis_t<real> xmat(1.0, 0.0, 0.0, 0.0, c, -s, 0.0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis_t<real> ymat(c, 0.0, s, 0.0, 1.0, 0.0, -s, 0.0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis_t<real> zmat(c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0);

	*this = xmat * zmat * ymat;
}
template<typename real>
Vector3_t<real> Basis_t<real>::get_euler_yzx() const {
	// Euler angles in YZX convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cy*cz             sy*sx-cy*cx*sz     cx*sy+cy*sz*sx
	//        sz                cz*cx              -cz*sx
	//        -cz*sy            cy*sx+cx*sy*sz     cy*cx-sy*sz*sx

	Vector3_t<real> euler;
	real sz = elements[1][0];
	if (sz < (1.0 - CMP_EPSILON)) {
		if (sz > -(1.0 - CMP_EPSILON)) {
			euler.x = atan2f(-elements[1][2], elements[1][1]);
			euler.y = atan2f(-elements[2][0], elements[0][0]);
			euler.z = asinf(sz);
		} else {
			// It's -1
			euler.x = atan2f(elements[2][1], elements[2][2]);
			euler.y = 0.0;
			euler.z = -PI / 2.0;
		}
	} else {
		// It's 1
		euler.x = atan2f(elements[2][1], elements[2][2]);
		euler.y = 0.0;
		euler.z = PI / 2.0;
	}
	return euler;
}
template<typename real>
void Basis_t<real>::set_euler_yzx(const Vector3_t<real> &p_euler) {
	real c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis_t<real> xmat(1.0, 0.0, 0.0, 0.0, c, -s, 0.0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis_t<real> ymat(c, 0.0, s, 0.0, 1.0, 0.0, -s, 0.0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis_t<real> zmat(c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0);

	*this = ymat * zmat * xmat;
}

// get_euler_yxz returns a vector containing the Euler angles in the YXZ convention,
// as in first-Z, then-X, last-Y. The angles for X, Y, and Z rotations are returned
// as the x, y, and z components of a Vector3_t<real> respectively.
template<typename real>
Vector3_t<real> Basis_t<real>::get_euler_yxz() const {
	// Euler angles in YXZ convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cy*cz+sy*sx*sz    cz*sy*sx-cy*sz        cx*sy
	//        cx*sz             cx*cz                 -sx
	//        cy*sx*sz-cz*sy    cy*cz*sx+sy*sz        cy*cx

	Vector3_t<real> euler;

	real m12 = elements[1][2];

	if (m12 < (1 - CMP_EPSILON)) {
		if (m12 > -(1 - CMP_EPSILON)) {
			// is this a pure X rotation?
			if (elements[1][0] == 0 && elements[0][1] == 0 && elements[0][2] == 0 && elements[2][0] == 0 && elements[0][0] == 1) {
				// return the simplest form (human friendlier in editor and scripts)
				euler.x = atan2(-m12, elements[1][1]);
				euler.y = 0;
				euler.z = 0;
			} else {
				euler.x = asin(-m12);
				euler.y = atan2(elements[0][2], elements[2][2]);
				euler.z = atan2(elements[1][0], elements[1][1]);
			}
		} else { // m12 == -1
			euler.x = PI * 0.5;
			euler.y = atan2(elements[0][1], elements[0][0]);
			euler.z = 0;
		}
	} else { // m12 == 1
		euler.x = -PI * 0.5;
		euler.y = -atan2(elements[0][1], elements[0][0]);
		euler.z = 0;
	}

	return euler;
}

// set_euler_yxz expects a vector containing the Euler angles in the format
// (ax,ay,az), where ax is the angle of rotation around x axis,
// and similar for other axes.
// The current implementation uses YXZ convention (Z is the first rotation).
template<typename real>
void Basis_t<real>::set_euler_yxz(const Vector3_t<real> &p_euler) {
	real c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis_t<real> xmat(1.0, 0.0, 0.0, 0.0, c, -s, 0.0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis_t<real> ymat(c, 0.0, s, 0.0, 1.0, 0.0, -s, 0.0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis_t<real> zmat(c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0);

	//optimizer will optimize away all this anyway
	*this = ymat * xmat * zmat;
}
template<typename real>
Vector3_t<real> Basis_t<real>::get_euler_zxy() const {
	// Euler angles in ZXY convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cz*cy-sz*sx*sy    -cx*sz                cz*sy+cy*sz*sx
	//        cy*sz+cz*sx*sy    cz*cx                 sz*sy-cz*cy*sx
	//        -cx*sy            sx                    cx*cy
	Vector3_t<real> euler;
	real sx = elements[2][1];
	if (sx < (1.0 - CMP_EPSILON)) {
		if (sx > -(1.0 - CMP_EPSILON)) {
			euler.x = asinf(sx);
			euler.y = atan2f(-elements[2][0], elements[2][2]);
			euler.z = atan2f(-elements[0][1], elements[1][1]);
		} else {
			// It's -1
			euler.x = -PI / 2.0;
			euler.y = atan2f(elements[0][2], elements[0][0]);
			euler.z = 0;
		}
	} else {
		// It's 1
		euler.x = PI / 2.0;
		euler.y = atan2f(elements[0][2], elements[0][0]);
		euler.z = 0;
	}
	return euler;
}
template<typename real>
void Basis_t<real>::set_euler_zxy(const Vector3_t<real> &p_euler) {
	real c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis_t<real> xmat(1.0, 0.0, 0.0, 0.0, c, -s, 0.0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis_t<real> ymat(c, 0.0, s, 0.0, 1.0, 0.0, -s, 0.0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis_t<real> zmat(c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0);

	*this = zmat * xmat * ymat;
}
template<typename real>
Vector3_t<real> Basis_t<real>::get_euler_zyx() const {
	// Euler angles in ZYX convention.
	// See https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	//
	// rot =  cz*cy             cz*sy*sx-cx*sz        sz*sx+cz*cx*cy
	//        cy*sz             cz*cx+sz*sy*sx        cx*sz*sy-cz*sx
	//        -sy               cy*sx                 cy*cx
	Vector3_t<real> euler;
	real sy = elements[2][0];
	if (sy < (1.0 - CMP_EPSILON)) {
		if (sy > -(1.0 - CMP_EPSILON)) {
			euler.x = atan2f(elements[2][1], elements[2][2]);
			euler.y = asinf(-sy);
			euler.z = atan2f(elements[1][0], elements[0][0]);
		} else {
			// It's -1
			euler.x = 0;
			euler.y = PI / 2.0;
			euler.z = -atan2f(elements[0][1], elements[1][1]);
		}
	} else {
		// It's 1
		euler.x = 0;
		euler.y = -PI / 2.0;
		euler.z = -atan2f(elements[0][1], elements[1][1]);
	}
	return euler;
}
template<typename real>
void Basis_t<real>::set_euler_zyx(const Vector3_t<real> &p_euler) {
	real c, s;

	c = cosf(p_euler.x);
	s = sinf(p_euler.x);
	Basis_t<real> xmat(1.0, 0.0, 0.0, 0.0, c, -s, 0.0, s, c);

	c = cosf(p_euler.y);
	s = sinf(p_euler.y);
	Basis_t<real> ymat(c, 0.0, s, 0.0, 1.0, 0.0, -s, 0.0, c);

	c = cosf(p_euler.z);
	s = sinf(p_euler.z);
	Basis_t<real> zmat(c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0);

	*this = zmat * ymat * xmat;
}
template<typename real>
bool Basis_t<real>::is_equal_approx(const Basis_t<real> &other) const {
	return elements[0].is_equal_approx(other.elements[0]) && elements[1].is_equal_approx(other.elements[1]) && elements[2].is_equal_approx(other.elements[2]);
}
template<typename real>
bool Basis_t<real>::is_equal_approx_ratio(const Basis_t<real> &a, const Basis_t<real> &b, real p_epsilon) const {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (!is_equal_approx_f(a.elements[i][j], b.elements[i][j])) {
				return false;
			}
		}
	}

	return true;
}
template<typename real>
bool Basis_t<real>::operator==(const Basis_t<real> &p_matrix) const {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (elements[i][j] != p_matrix.elements[i][j]) {
				return false;
			}
		}
	}

	return true;
}
template<typename real>
bool Basis_t<real>::operator!=(const Basis_t<real> &p_matrix) const {
	return (!(*this == p_matrix));
}
// template<typename real>
// Quat Basis_t<real>::get_quat() const {
// #ifdef MATH_CHECKS
// 	ERR_FAIL_COND_V_MSG(!is_rotation(), Quat(), "Basis_t<real> must be normalized in order to be casted to a Quaternion. Use get_rotation_quat() or call orthonormalized() if the Basis_t<real> contains linearly independent vectors.");
// #endif
// 	/* Allow getting a quaternion from an unnormalized transform */
// 	Basis_t<real> m = *this;
// 	real trace = m.elements[0][0] + m.elements[1][1] + m.elements[2][2];
// 	real temp[4];

// 	if (trace > 0.0) {
// 		real s = sqrtf(trace + 1.0);
// 		temp[3] = (s * 0.5);
// 		s = 0.5 / s;

// 		temp[0] = ((m.elements[2][1] - m.elements[1][2]) * s);
// 		temp[1] = ((m.elements[0][2] - m.elements[2][0]) * s);
// 		temp[2] = ((m.elements[1][0] - m.elements[0][1]) * s);
// 	} else {
// 		int i = m.elements[0][0] < m.elements[1][1]
// 				? (m.elements[1][1] < m.elements[2][2] ? 2 : 1)
// 				: (m.elements[0][0] < m.elements[2][2] ? 2 : 0);
// 		int j = (i + 1) % 3;
// 		int k = (i + 2) % 3;

// 		real s = sqrtf(m.elements[i][i] - m.elements[j][j] - m.elements[k][k] + 1.0);
// 		temp[i] = s * 0.5;
// 		s = 0.5 / s;

// 		temp[3] = (m.elements[k][j] - m.elements[j][k]) * s;
// 		temp[j] = (m.elements[j][i] + m.elements[i][j]) * s;
// 		temp[k] = (m.elements[k][i] + m.elements[i][k]) * s;
// 	}

// 	return Quat(temp[0], temp[1], temp[2], temp[3]);
// }
template<typename real>
static Basis_t<real> get_ortho_bases(const int index) {
    static const Basis_t<real> _ortho_bases[24] = {
        Basis_t<real>(1, 0, 0, 0, 1, 0, 0, 0, 1),
        Basis_t<real>(0, -1, 0, 1, 0, 0, 0, 0, 1),
        Basis_t<real>(-1, 0, 0, 0, -1, 0, 0, 0, 1),
        Basis_t<real>(0, 1, 0, -1, 0, 0, 0, 0, 1),
        Basis_t<real>(1, 0, 0, 0, 0, -1, 0, 1, 0),
        Basis_t<real>(0, 0, 1, 1, 0, 0, 0, 1, 0),
        Basis_t<real>(-1, 0, 0, 0, 0, 1, 0, 1, 0),
        Basis_t<real>(0, 0, -1, -1, 0, 0, 0, 1, 0),
        Basis_t<real>(1, 0, 0, 0, -1, 0, 0, 0, -1),
        Basis_t<real>(0, 1, 0, 1, 0, 0, 0, 0, -1),
        Basis_t<real>(-1, 0, 0, 0, 1, 0, 0, 0, -1),
        Basis_t<real>(0, -1, 0, -1, 0, 0, 0, 0, -1),
        Basis_t<real>(1, 0, 0, 0, 0, 1, 0, -1, 0),
        Basis_t<real>(0, 0, -1, 1, 0, 0, 0, -1, 0),
        Basis_t<real>(-1, 0, 0, 0, 0, -1, 0, -1, 0),
        Basis_t<real>(0, 0, 1, -1, 0, 0, 0, -1, 0),
        Basis_t<real>(0, 0, 1, 0, 1, 0, -1, 0, 0),
        Basis_t<real>(0, -1, 0, 0, 0, 1, -1, 0, 0),
        Basis_t<real>(0, 0, -1, 0, -1, 0, -1, 0, 0),
        Basis_t<real>(0, 1, 0, 0, 0, -1, -1, 0, 0),
        Basis_t<real>(0, 0, 1, 0, -1, 0, 1, 0, 0),
        Basis_t<real>(0, 1, 0, 0, 0, 1, 1, 0, 0),
        Basis_t<real>(0, 0, -1, 0, 1, 0, 1, 0, 0),
        Basis_t<real>(0, -1, 0, 0, 0, -1, 1, 0, 0)
    };
    return _ortho_bases[index];
}

template<typename real>
int Basis_t<real>::get_orthogonal_index() const {
	//could be sped up if i come up with a way
	Basis_t<real> orth = *this;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			real v = orth[i][j];
			if (v > 0.5) {
				v = 1.0;
			} else if (v < -0.5) {
				v = -1.0;
			} else {
				v = 0;
			}

			orth[i][j] = v;
		}
	}

	for (int i = 0; i < 24; i++) {
		if (get_ortho_bases<real>(i) == orth) {
			return i;
		}
	}

	return 0;
}
template<typename real>
void Basis_t<real>::set_orthogonal_index(int p_index) {

	*this = get_ortho_bases<real>(p_index);
}
template<typename real>
void Basis_t<real>::get_axis_angle(Vector3_t<real> &r_axis, real &r_angle) const {
	/* checking this is a bad idea, because obtaining from scaled transform is a valid use case
#ifdef MATH_CHECKS
	ERR_FAIL_COND(!is_rotation());
#endif
*/
	real angle, x, y, z; // variables for result
	real epsilon = 0.01; // margin to allow for rounding errors
	real epsilon2 = 0.1; // margin to distinguish between 0 and 180 degrees

	if ((fabs(elements[1][0] - elements[0][1]) < epsilon) && (fabs(elements[2][0] - elements[0][2]) < epsilon) && (fabs(elements[2][1] - elements[1][2]) < epsilon)) {
		// singularity found
		// first check for identity matrix which must have +1 for all terms
		//  in leading diagonaland zero in other terms
		if ((fabs(elements[1][0] + elements[0][1]) < epsilon2) && (fabs(elements[2][0] + elements[0][2]) < epsilon2) && (fabs(elements[2][1] + elements[1][2]) < epsilon2) && (fabs(elements[0][0] + elements[1][1] + elements[2][2] - 3) < epsilon2)) {
			// this singularity is identity matrix so angle = 0
			r_axis = Vector3_t<real>(0, 1, 0);
			r_angle = 0;
			return;
		}
		// otherwise this singularity is angle = 180
		angle = PI;
		real xx = (elements[0][0] + 1) / 2;
		real yy = (elements[1][1] + 1) / 2;
		real zz = (elements[2][2] + 1) / 2;
		real xy = (elements[1][0] + elements[0][1]) / 4;
		real xz = (elements[2][0] + elements[0][2]) / 4;
		real yz = (elements[2][1] + elements[1][2]) / 4;
		if ((xx > yy) && (xx > zz)) { // elements[0][0] is the largest diagonal term
			if (xx < epsilon) {
				x = 0;
				y = SQRT12;
				z = SQRT12;
			} else {
				x = sqrtf(xx);
				y = xy / x;
				z = xz / x;
			}
		} else if (yy > zz) { // elements[1][1] is the largest diagonal term
			if (yy < epsilon) {
				x = SQRT12;
				y = 0;
				z = SQRT12;
			} else {
				y = sqrtf(yy);
				x = xy / y;
				z = yz / y;
			}
		} else { // elements[2][2] is the largest diagonal term so base result on this
			if (zz < epsilon) {
				x = SQRT12;
				y = SQRT12;
				z = 0;
			} else {
				z = sqrtf(zz);
				x = xz / z;
				y = yz / z;
			}
		}
		r_axis = Vector3_t<real>(x, y, z);
		r_angle = angle;
		return;
	}
	// as we have reached here there are no singularities so we can handle normally
	real s = sqrtf((elements[1][2] - elements[2][1]) * (elements[1][2] - elements[2][1]) + (elements[2][0] - elements[0][2]) * (elements[2][0] - elements[0][2]) + (elements[0][1] - elements[1][0]) * (elements[0][1] - elements[1][0])); // s=|axis||sin(angle)|, used to normalise

	angle = acosf((elements[0][0] + elements[1][1] + elements[2][2] - 1) / 2);
	if (angle < 0) {
		s = -s;
	}
	x = (elements[2][1] - elements[1][2]) / s;
	y = (elements[0][2] - elements[2][0]) / s;
	z = (elements[1][0] - elements[0][1]) / s;

	r_axis = Vector3_t<real>(x, y, z);
	r_angle = angle;
}
// template<typename real>
// void Basis_t<real>::set_quat(const Quat &p_quat) {
// 	real d = p_quat.length_squared();
// 	real s = 2.0 / d;
// 	real xs = p_quat.x * s, ys = p_quat.y * s, zs = p_quat.z * s;
// 	real wx = p_quat.w * xs, wy = p_quat.w * ys, wz = p_quat.w * zs;
// 	real xx = p_quat.x * xs, xy = p_quat.x * ys, xz = p_quat.x * zs;
// 	real yy = p_quat.y * ys, yz = p_quat.y * zs, zz = p_quat.z * zs;
// 	set(1.0 - (yy + zz), xy - wz, xz + wy,
// 			xy + wz, 1.0 - (xx + zz), yz - wx,
// 			xz - wy, yz + wx, 1.0 - (xx + yy));
// }
template<typename real>
void Basis_t<real>::set_axis_angle(const Vector3_t<real> &p_axis, real p_phi) {
// Rotation matrix from axis and angle, see https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_angle
#ifdef MATH_CHECKS
	ERR_FAIL_COND_MSG(!p_axis.is_normalized(), "The axis Vector3_t<real> must be normalized.");
#endif
	Vector3_t<real> axis_sq(p_axis.x * p_axis.x, p_axis.y * p_axis.y, p_axis.z * p_axis.z);
	real cosine = cosf(p_phi);
	elements[0][0] = axis_sq.x + cosine * (1.0 - axis_sq.x);
	elements[1][1] = axis_sq.y + cosine * (1.0 - axis_sq.y);
	elements[2][2] = axis_sq.z + cosine * (1.0 - axis_sq.z);

	real sine = sinf(p_phi);
	real _t = 1 - cosine;

	real xyzt = p_axis.x * p_axis.y * _t;
	real zyxs = p_axis.z * sine;
	elements[0][1] = xyzt - zyxs;
	elements[1][0] = xyzt + zyxs;

	xyzt = p_axis.x * p_axis.z * _t;
	zyxs = p_axis.y * sine;
	elements[0][2] = xyzt + zyxs;
	elements[2][0] = xyzt - zyxs;

	xyzt = p_axis.y * p_axis.z * _t;
	zyxs = p_axis.x * sine;
	elements[1][2] = xyzt - zyxs;
	elements[2][1] = xyzt + zyxs;
}
template<typename real>
void Basis_t<real>::set_axis_angle_scale(const Vector3_t<real> &p_axis, real p_phi, const Vector3_t<real> &p_scale) {
	set_diagonal(p_scale);
	rotate(p_axis, p_phi);
}
template<typename real>
void Basis_t<real>::set_euler_scale(const Vector3_t<real> &p_euler, const Vector3_t<real> &p_scale) {
	set_diagonal(p_scale);
	rotate(p_euler);
}
// template<typename real>
// void Basis_t<real>::set_quat_scale(const Quat &p_quat, const Vector3_t<real> &p_scale) {
// 	set_diagonal(p_scale);
// 	rotate(p_quat);
// }
template<typename real>
void Basis_t<real>::set_diagonal(const Vector3_t<real> &p_diag) {
	elements[0][0] = p_diag.x;
	elements[0][1] = 0;
	elements[0][2] = 0;

	elements[1][0] = 0;
	elements[1][1] = p_diag.y;
	elements[1][2] = 0;

	elements[2][0] = 0;
	elements[2][1] = 0;
	elements[2][2] = p_diag.z;
}
// template<typename real>
// Basis_t<real> Basis_t<real>::slerp(const Basis_t<real> &p_to, const real &p_weight) const {
// 	//consider scale
// 	Quat from(*this);
// 	Quat to(p_to);

// 	Basis_t<real> b(from.slerp(to, p_weight));
// 	b.elements[0] *= lerp(elements[0].length(), p_to.elements[0].length(), p_weight);
// 	b.elements[1] *= lerp(elements[1].length(), p_to.elements[1].length(), p_weight);
// 	b.elements[2] *= lerp(elements[2].length(), p_to.elements[2].length(), p_weight);

// 	return b;
// }
