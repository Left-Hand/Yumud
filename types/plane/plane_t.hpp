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


#include "vector3/vector3_t.hpp"
#include <optional>

struct Plane {
	Vector3 normal;
	real_t d = 0;

	void set_normal(const Vector3 &p_normal);
	__fast_inline Vector3 get_normal() const { return normal; };

	void normalize();
	Plane normalized() const;

	/* Plane-Point operations */

	__fast_inline Vector3 get_center() const { return normal * d; }
	Vector3 get_any_perpendicular_normal() const;

	__fast_inline bool is_point_over(const Vector3 &p_point) const; ///< Point is over plane
	__fast_inline real_t distance_to(const Vector3 &p_point) const;
	__fast_inline bool has_point(const Vector3 &p_point, real_t p_tolerance = real_t(CMP_EPSILON)) const;

	/* intersections */

	bool intersect_3(const Plane &p_plane1, const Plane &p_plane2, Vector3 *r_result = nullptr) const;
	bool intersects_ray(const Vector3 &p_from, const Vector3 &p_dir, Vector3 *p_intersection) const;
	bool intersects_segment(const Vector3 &p_begin, const Vector3 &p_end, Vector3 *p_intersection) const;

	// For Variant bindings.
	std::optional<Vector3> intersect_3_bind(const Plane &p_plane1, const Plane &p_plane2) const;
	std::optional<Vector3> intersects_ray_bind(const Vector3 &p_from, const Vector3 &p_dir) const;
	std::optional<Vector3> intersects_segment_bind(const Vector3 &p_begin, const Vector3 &p_end) const;

	__fast_inline Vector3 project(const Vector3 &p_point) const {
		return p_point - normal * distance_to(p_point);
	}

	/* misc */

	// Plane operator-() const { return Plane(Vector3(-normal), real_t(-d));}
	bool is_equal_approx(const Plane &p_plane) const;
	bool is_equal_approx_any_side(const Plane &p_plane) const;
	bool is_finite() const;

	__fast_inline bool operator==(const Plane &p_plane) const;
	__fast_inline bool operator!=(const Plane &p_plane) const;

	__fast_inline Plane() {}
	__fast_inline Plane(real_t p_a, real_t p_b, real_t p_c, real_t p_d) :
			normal(p_a, p_b, p_c),
			d(p_d) {}

	__fast_inline Plane(const Vector3 &p_normal, real_t p_d = real_t(0));
	__fast_inline Plane(const Vector3 &p_normal, const Vector3 &p_point);
	__fast_inline Plane(const Vector3 &p_point1, const Vector3 &p_point2, const Vector3 &p_point3, ClockDirection p_dir = CLOCKWISE);
};

bool Plane::is_point_over(const Vector3 &p_point) const {
	return (normal.dot(p_point) > d);
}

real_t Plane::distance_to(const Vector3 &p_point) const {
	return (normal.dot(p_point) - d);
}

bool Plane::has_point(const Vector3 &p_point, real_t p_tolerance) const {
	real_t dist = normal.normalized().dot(p_point) - d;
	dist = ABS(dist);
	return (dist <= p_tolerance);
}

Plane::Plane(const Vector3 &p_normal, real_t p_d) :
		normal(p_normal.normalized()),
		d(p_d) {
}

Plane::Plane(const Vector3 &p_normal, const Vector3 &p_point) :
		normal(p_normal.normalized()),
		d(p_normal.dot(p_point)) {
}

Plane::Plane(const Vector3 &p_point1, const Vector3 &p_point2, const Vector3 &p_point3, ClockDirection p_dir) {
	if (p_dir == CLOCKWISE) {
		normal = (p_point1 - p_point3).cross(p_point1 - p_point2);
	} else {
		normal = (p_point1 - p_point2).cross(p_point1 - p_point3);
	}

	normal.normalize();
	d = normal.dot(p_point1);
}

bool Plane::operator==(const Plane &p_plane) const {
	return normal == p_plane.normal && d == p_plane.d;
}

bool Plane::operator!=(const Plane &p_plane) const {
	return normal != p_plane.normal || d != p_plane.d;
}

#include "plane_t.tpp"


__fast_inline OutputStream & operator<<(OutputStream & os, const Plane & value){
    return os << '(' << value.normal << ',' << value.d << ')';
}