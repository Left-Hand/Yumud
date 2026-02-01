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

namespace ymd{

template<arithmetic T>
struct Plane {
	math::Vec3<T> normal;
	T d = 0;


	__fast_inline constexpr Plane(const math::Vec3<auto> & p_normal, const arithmetic auto p_d) :
			normal(p_normal),
			d(p_d) {
	}

	__fast_inline constexpr Plane(const math::Vec3<auto> & p_normal, const math::Vec3<auto> & p_point) :
			normal(p_normal),
			d(p_normal.dot(p_point)) {
	}

	__fast_inline constexpr Plane(const math::Vec3<auto> &p_point1, const math::Vec3<auto> &p_point2, const math::Vec3<auto> &p_point3,const RotateDirection p_dir = CW) {
		if (p_dir == CW) {
			normal = (p_point1 - p_point3).cross(p_point1 - p_point2);
		} else {
			normal = (p_point1 - p_point2).cross(p_point1 - p_point3);
		}

		normal.normalize();
		d = normal.dot(p_point1);
	}
	
	void set_normal(const math::Vec3<auto> &p_normal);
	__fast_inline math::Vec3<T> get_normal() const { return normal; };

	void normalize();
	Plane normalized() const;

	/* Plane-Point operations */

	__fast_inline math::Vec3<T> get_center() const { return normal * d; }
	math::Vec3<T> get_any_perpendicular_normal() const;

	__fast_inline bool is_point_over(const math::Vec3<T> &p_point) const; ///< Point is over plane
	__fast_inline T distance_to(const math::Vec3<T> &p_point) const;
	__fast_inline bool has_point(const math::Vec3<T> &p_point,const T p_tolerance = static_cast<T>(CMP_EPSILON)) const;

	/* intersections */

	bool intersect_3(const Plane & p_plane1, const Plane & p_plane2, math::Vec3<T> & r_result) const;
	bool intersects_ray(const math::Vec3<T> &p_from, const math::Vec3<T> &p_dir, math::Vec3<T> & p_intersection) const;
	bool intersects_segment(const math::Vec3<T> &p_begin, const math::Vec3<T> &p_end, math::Vec3<T> & p_intersection) const;

	// For Variant bindings.
	std::optional<math::Vec3<T>> intersect_3(const Plane & p_plane1, const Plane & p_plane2) const;
	std::optional<math::Vec3<T>> intersects_ray(const math::Vec3<T> &p_from, const math::Vec3<T> &p_dir) const;
	std::optional<math::Vec3<T>> intersects_segment(const math::Vec3<T> &p_begin, const math::Vec3<T> &p_end) const;

	__fast_inline math::Vec3<T> project(const math::Vec3<T> &p_point) const {
		return p_point - normal * distance_to(p_point);
	}

	/* misc */

	// Plane operator-() const { return Plane(math::Vec3<T>(-normal), static_cast<T>(-d));}
	bool is_equal_approx(const Plane & p_plane) const;
	bool is_equal_approx_any_side(const Plane & p_plane) const;
	bool is_finite() const;

	__fast_inline bool operator==(const Plane & p_plane) const;
	__fast_inline bool operator!=(const Plane & p_plane) const;

	__fast_inline Plane() {}
	__fast_inline Plane(const auto & p_a, const auto & p_b, const auto & p_c, const auto & p_d) :
			normal(math::Vec3<T>{p_a, p_b, p_c}),
			d(static_cast<T>(p_d)) {}

};

template<arithmetic T>
bool Plane<T>::is_point_over(const math::Vec3<T> &p_point) const {
	return (normal.dot(p_point) > d);
}

template<arithmetic T>
T Plane<T>::distance_to(const math::Vec3<T> &p_point) const {
	return (normal.dot(p_point) - d);
}

template<arithmetic T>
bool Plane<T>::has_point(const math::Vec3<T> &p_point, const T p_tolerance) const {
	T dist = normal.normalized().dot(p_point) - d;
	dist = ABS(dist);
	return (dist <= p_tolerance);
}



template<arithmetic T>
bool Plane<T>::operator==(const Plane<T> &p_plane) const {
	return normal == p_plane.normal && d == p_plane.d;
}

template<arithmetic T>
bool Plane<T>::operator!=(const Plane<T> &p_plane) const {
	return normal != p_plane.normal || d != p_plane.d;
}

using PlaneR = Plane<real_t>;
using PlaneF = Plane<float>;
using PlaneD = Plane<double>;

template<arithmetic T>
__fast_inline OutputStream & operator<<(OutputStream & os, const Plane<T> & value){
    return os << os.brackets<'('>() << 
		value.normal << os.splitter() << 
		value.d << os.brackets<')'>();
}


}


namespace ymd{

template<arithmetic T>
void Plane<T>::set_normal(const math::Vec3<auto> &p_normal) {
	normal = p_normal;
}


template<arithmetic T>
void Plane<T>::normalize() {
	T l = normal.length();
	if (l == 0) {
		*this = Plane<T>(0, 0, 0, 0);
		return;
	}
	normal /= l;
	d /= l;
}


template<arithmetic T>
Plane<T> Plane<T>::normalized() const {
	Plane<T> p = *this;
	p.normalize();
	return p;
}


template<arithmetic T>
math::Vec3<T> Plane<T>::get_any_perpendicular_normal() const {
	static constexpr auto p1 = math::Vec3<T>(1, 0, 0);
	static constexpr auto p2 = math::Vec3<T>(0, 1, 0);
	math::Vec3<T> p;

	if (ABS(normal.dot(p1)) > static_cast<T>(0.99f)) { // if too similar to p1
		p = p2; // use p2
	} else {
		p = p1; // use p1
	}

	p -= normal * normal.dot(p);
	p.normalize();

	return p;
}

/* intersections */

template<arithmetic T>
bool Plane<T>::intersect_3(const Plane<T> &p_plane1, const Plane<T> &p_plane2, math::Vec3<T> & r_result) const {
	const Plane<T> &p_plane0 = *this;
	math::Vec3<T> normal0 = p_plane0.normal;
	math::Vec3<T> normal1 = p_plane1.normal;
	math::Vec3<T> normal2 = p_plane2.normal;

	T denom = (normal0.corss(normal1)).dot(normal2);

	if (is_equal_approx(denom, 0)) {
		return false;
	}

    r_result = ((normal1.cross(normal2) * p_plane0.d) +
                        (normal2.cross(normal0) * p_plane1.d) +
                        ((normal0.cross(normal1) * p_plane2.d)) / denom);

	return true;
}



template<arithmetic T>
bool Plane<T>::intersects_ray(const math::Vec3<T> &p_from, const math::Vec3<T> &p_dir, math::Vec3<T> & p_intersection) const {
	math::Vec3<T> segment = p_dir;
	T den = normal.dot(segment);

	if (is_equal_approx(0, den)) {
		return false;
	}

	T dist = (normal.dot(p_from) - d) / den;

	if (dist > (T)CMP_EPSILON) { //this is a ray, before the emitting pos (p_from) doesn't exist

		return false;
	}

	dist = -dist;
	p_intersection = p_from + segment * dist;

	return true;
}


template<arithmetic T>
bool Plane<T>::intersects_segment(const math::Vec3<T> &p_begin, const math::Vec3<T> &p_end, math::Vec3<T> & p_intersection) const {
	math::Vec3<T> segment = p_begin - p_end;
	T den = normal.dot(segment);
	if (is_equal_approx(0, den)) {
		return false;
	}

	T dist = (normal.dot(p_begin) - d) / den;
	if (dist < static_cast<T>(-CMP_EPSILON) or dist > static_cast<T>(1.0f + CMP_EPSILON)) {
		return false;
	}

	dist = -dist;
	p_intersection = p_begin + segment * dist;

	return true;
}


template<arithmetic T>
std::optional<math::Vec3<T>> Plane<T>::intersect_3(const Plane<T> &p_plane1, const Plane<T> &p_plane2) const {
	math::Vec3<T> inters;
	if (intersect_3(p_plane1, p_plane2, inters)) {
		return inters;
	} else {
		return std::nullopt;
	}
}


template<arithmetic T>
std::optional<math::Vec3<T>> Plane<T>::intersects_ray(const math::Vec3<T> &p_from, const math::Vec3<T> &p_dir) const {
	math::Vec3<T> inters;
	if (intersects_ray(p_from, p_dir, inters)) {
		return inters;
	} else {
		return std::nullopt;
	}
}


template<arithmetic T>
std::optional<math::Vec3<T>> Plane<T>::intersects_segment(const math::Vec3<T> &p_begin, const math::Vec3<T> &p_end) const {
	math::Vec3<T> inters;
	if (intersects_segment(p_begin, p_end, inters)) {
		return inters;
	} else {
		return std::nullopt;
	}
}

/* misc */

template<arithmetic T>
bool Plane<T>::is_equal_approx_any_side(const Plane<T> &p_plane) const {
	return (normal.is_equal_approx(p_plane.normal) && is_equal_approx(d, p_plane.d)) || (normal.is_equal_approx(-p_plane.normal) && is_equal_approx(d, -p_plane.d));
}


template<arithmetic T>
bool Plane<T>::is_equal_approx(const Plane<T> &p_plane) const {
	return normal.is_equal_approx(p_plane.normal) && is_equal_approx(d, p_plane.d);
}


template<arithmetic T>
bool Plane<T>::is_finite() const {
	return normal.is_finite() && is_finite(d);
}


}