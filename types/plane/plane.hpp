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


#include "types/vector3/vector3.hpp"

namespace ymd{

template<arithmetic T>
struct Plane_t {
	Vector3_t<T> normal;
	T d = 0;


	__fast_inline constexpr Plane_t(const Vector3_t<auto> & p_normal, const arithmetic auto p_d) :
			normal(p_normal),
			d(p_d) {
	}

	__fast_inline constexpr Plane_t(const Vector3_t<auto> & p_normal, const Vector3_t<auto> & p_point) :
			normal(p_normal),
			d(p_normal.dot(p_point)) {
	}

	__fast_inline constexpr Plane_t(const Vector3_t<auto> &p_point1, const Vector3_t<auto> &p_point2, const Vector3_t<auto> &p_point3,const ClockDirection p_dir = CLOCKWISE) {
		if (p_dir == CLOCKWISE) {
			normal = (p_point1 - p_point3).cross(p_point1 - p_point2);
		} else {
			normal = (p_point1 - p_point2).cross(p_point1 - p_point3);
		}

		normal.normalize();
		d = normal.dot(p_point1);
	}
	
	void set_normal(const Vector3_t<auto> &p_normal);
	__fast_inline Vector3_t<T> get_normal() const { return normal; };

	void normalize();
	Plane_t normalized() const;

	/* Plane-Point operations */

	__fast_inline Vector3_t<T> get_center() const { return normal * d; }
	Vector3_t<T> get_any_perpendicular_normal() const;

	__fast_inline bool is_point_over(const Vector3_t<T> &p_point) const; ///< Point is over plane
	__fast_inline T distance_to(const Vector3_t<T> &p_point) const;
	__fast_inline bool has_point(const Vector3_t<T> &p_point,const T p_tolerance = static_cast<T>(CMP_EPSILON)) const;

	/* intersections */

	bool intersect_3(const Plane_t & p_plane1, const Plane_t & p_plane2, Vector3_t<T> & r_result) const;
	bool intersects_ray(const Vector3_t<T> &p_from, const Vector3_t<T> &p_dir, Vector3_t<T> & p_intersection) const;
	bool intersects_segment(const Vector3_t<T> &p_begin, const Vector3_t<T> &p_end, Vector3_t<T> & p_intersection) const;

	// For Variant bindings.
	std::optional<Vector3_t<T>> intersect_3(const Plane_t & p_plane1, const Plane_t & p_plane2) const;
	std::optional<Vector3_t<T>> intersects_ray(const Vector3_t<T> &p_from, const Vector3_t<T> &p_dir) const;
	std::optional<Vector3_t<T>> intersects_segment(const Vector3_t<T> &p_begin, const Vector3_t<T> &p_end) const;

	__fast_inline Vector3_t<T> project(const Vector3_t<T> &p_point) const {
		return p_point - normal * distance_to(p_point);
	}

	/* misc */

	// Plane operator-() const { return Plane(Vector3_t<T>(-normal), static_cast<T>(-d));}
	bool is_equal_approx(const Plane_t & p_plane) const;
	bool is_equal_approx_any_side(const Plane_t & p_plane) const;
	bool is_finite() const;

	__fast_inline bool operator==(const Plane_t & p_plane) const;
	__fast_inline bool operator!=(const Plane_t & p_plane) const;

	__fast_inline Plane_t() {}
	__fast_inline Plane_t(const auto & p_a, const auto & p_b, const auto & p_c, const auto & p_d) :
			normal(Vector3_t<T>{p_a, p_b, p_c}),
			d(static_cast<T>(p_d)) {}

};

template<arithmetic T>
bool Plane_t<T>::is_point_over(const Vector3_t<T> &p_point) const {
	return (normal.dot(p_point) > d);
}

template<arithmetic T>
T Plane_t<T>::distance_to(const Vector3_t<T> &p_point) const {
	return (normal.dot(p_point) - d);
}

template<arithmetic T>
bool Plane_t<T>::has_point(const Vector3_t<T> &p_point, const T p_tolerance) const {
	T dist = normal.normalized().dot(p_point) - d;
	dist = ABS(dist);
	return (dist <= p_tolerance);
}



template<arithmetic T>
bool Plane_t<T>::operator==(const Plane_t<T> &p_plane) const {
	return normal == p_plane.normal && d == p_plane.d;
}

template<arithmetic T>
bool Plane_t<T>::operator!=(const Plane_t<T> &p_plane) const {
	return normal != p_plane.normal || d != p_plane.d;
}

using PlaneR = Plane_t<real_t>;
using PlaneF = Plane_t<float>;
using PlaneD = Plane_t<double>;

template<arithmetic T>
__fast_inline OutputStream & operator<<(OutputStream & os, const Plane_t<T> & value){
    return os << os.brackets<'('>() << 
		value.normal << os.splitter() << 
		value.d << os.brackets<')'>();
}


}


#include "plane.tpp"