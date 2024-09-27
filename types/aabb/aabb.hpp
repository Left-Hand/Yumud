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

#ifndef AABB_H
#define AABB_H

#include "types/plane/plane_t.hpp"
#include "types/vector3/vector3_t.hpp"

/**
 * AABB_t (Axis Aligned Bounding Box)
 * This is implemented by a point (position) and the box size.
 */


template<arithmetic T>
struct AABB_t{
	Vector3_t<T> position;
	Vector3_t<T> size;

	T get_volume() const;
	__fast_inline bool has_volume() const {
		return size.x > 0 and size.y > 0 and size.z > 0;
	}

	__fast_inline bool has_surface() const {
		return size.x > 0 or size.y > 0 or size.z > 0;
	}

	const Vector3_t<T> & get_position() const { return position; }
	void set_position(const Vector3_t<T> & p_pos) { position = p_pos; }
	const Vector3_t<T> & get_size() const { return size; }
	void set_size(const Vector3_t<T> & p_size) { size = p_size; }

	bool operator==(const AABB_t<T> &p_rval) const;
	bool operator!=(const AABB_t<T> &p_rval) const;

	bool is_equal_approx(const AABB_t<T> &p_aabb) const;
	bool is_finite() const;
	__fast_inline bool intersects(const AABB_t<T> &p_aabb) const; /// Both AABBs overlap
	__fast_inline bool intersects_inclusive(const AABB_t<T> &p_aabb) const; /// Both AABBs (or their faces) overlap
	__fast_inline bool encloses(const AABB_t<T> &p_aabb) const; /// p_aabb is completely inside this

	AABB_t merge(const AABB_t<T> &p_with) const;
	void merge_with(const AABB_t<T> &p_aabb); ///merge with another AABB_t
	AABB_t intersection(const AABB_t<T> &p_aabb) const; ///get box where two intersect, empty if no intersection occurs
	__fast_inline bool smits_intersect_ray(const Vector3_t<T> & p_from, const Vector3_t<T> & p_dir, T p_t0, T p_t1) const;

	bool intersects_segment(const Vector3_t<T> & p_from, const Vector3_t<T> & p_to, Vector3_t<T> *r_intersection_point = nullptr, Vector3_t<T> *r_normal = nullptr) const;
	bool intersects_ray(const Vector3_t<T> & p_from, const Vector3_t<T> & p_dir) const {
		bool inside;
		return find_intersects_ray(p_from, p_dir, inside);
	}
	bool find_intersects_ray(const Vector3_t<T> & p_from, const Vector3_t<T> & p_dir, bool &r_inside, Vector3_t<T> *r_intersection_point = nullptr, Vector3_t<T> *r_normal = nullptr) const;

	__fast_inline bool intersects_convex_shape(const Plane_t<T> *p_planes, int p_plane_count, const Vector3_t<T> *p_points, int p_point_count) const;
	__fast_inline bool inside_convex_shape(const Plane_t<T> *p_planes, int p_plane_count) const;
	bool intersects_plane(const Plane_t<T> &p_plane) const;

	__fast_inline bool has_point(const Vector3_t<T> & p_point) const;
	__fast_inline Vector3_t<T> get_support(const Vector3_t<T> & p_normal) const;

	Vector3_t<T> get_longest_axis() const;
	int get_longest_axis_index() const;
	__fast_inline T get_longest_axis_size() const;

	Vector3_t<T> get_shortest_axis() const;
	int get_shortest_axis_index() const;
	__fast_inline T get_shortest_axis_size() const;

	AABB_t grow(T p_by) const;
	__fast_inline void grow_by(T p_amount);

	void get_edge(int p_edge, Vector3_t<T> &r_from, Vector3_t<T> &r_to) const;
	__fast_inline Vector3_t<T> get_endpoint(int p_point) const;

	AABB_t expand(const Vector3_t<T> & p_vector) const;
	__fast_inline void project_range_in_plane(const Plane_t<T> &p_plane, T &r_min, T &r_max) const;
	__fast_inline void expand_to(const Vector3_t<T> & p_vector); /** expand to contain a point if necessary */

	__fast_inline AABB_t abs() const {
		return AABB_t(position + size.minf(0), size.abs());
	}

	// Variant intersects_segment_bind(const Vector3_t<T> & p_from, const Vector3_t<T> & p_to) const;
	// Variant intersects_ray_bind(const Vector3_t<T> & p_from, const Vector3_t<T> & p_dir) const;

	__fast_inline void quantize(T p_unit);
	__fast_inline AABB_t quantized(T p_unit) const;

	__fast_inline void set_end(const Vector3_t<T> & p_end) {
		size = p_end - position;
	}

	__fast_inline Vector3_t<T> get_end() const {
		return position + size;
	}

	__fast_inline Vector3_t<T> get_center() const {
		return position + (size * T(static_cast<T>(0.5f)));
	}

	operator String() const;

	__fast_inline constexpr AABB_t() {}
	__fast_inline constexpr AABB_t(const Vector3_t<T> & p_pos, const Vector3_t<T> & p_size) :
			position(p_pos),
			size(p_size) {
	}
};

template<arithmetic T>
inline bool AABB_t<T>::intersects(const AABB_t<T> &p_aabb) const {
	if (position.x >= (p_aabb.position.x + p_aabb.size.x)) {
		return false;
	}
	if ((position.x + size.x) <= p_aabb.position.x) {
		return false;
	}
	if (position.y >= (p_aabb.position.y + p_aabb.size.y)) {
		return false;
	}
	if ((position.y + size.y) <= p_aabb.position.y) {
		return false;
	}
	if (position.z >= (p_aabb.position.z + p_aabb.size.z)) {
		return false;
	}
	if ((position.z + size.z) <= p_aabb.position.z) {
		return false;
	}

	return true;
}


template<arithmetic T>
inline bool AABB_t<T>::intersects_inclusive(const AABB_t<T> &p_aabb) const {
	if (position.x > (p_aabb.position.x + p_aabb.size.x)) {
		return false;
	}
	if ((position.x + size.x) < p_aabb.position.x) {
		return false;
	}
	if (position.y > (p_aabb.position.y + p_aabb.size.y)) {
		return false;
	}
	if ((position.y + size.y) < p_aabb.position.y) {
		return false;
	}
	if (position.z > (p_aabb.position.z + p_aabb.size.z)) {
		return false;
	}
	if ((position.z + size.z) < p_aabb.position.z) {
		return false;
	}

	return true;
}

template<arithmetic T>
inline bool AABB_t<T>::encloses(const AABB_t<T> &p_aabb) const {
	Vector3_t<T> src_min = position;
	Vector3_t<T> src_max = position + size;
	Vector3_t<T> dst_min = p_aabb.position;
	Vector3_t<T> dst_max = p_aabb.position + p_aabb.size;

	return (
			(src_min.x <= dst_min.x) and
			(src_max.x >= dst_max.x) and
			(src_min.y <= dst_min.y) and
			(src_max.y >= dst_max.y) and
			(src_min.z <= dst_min.z) and
			(src_max.z >= dst_max.z));
}

template<arithmetic T>
Vector3_t<T> AABB_t<T>::get_support(const Vector3_t<T> & p_normal) const {
	Vector3_t<T> half_extents = size * static_cast<T>(0.5f);
	Vector3_t<T> ofs = position + half_extents;

	return Vector3_t<T>(
				   (p_normal.x > 0) ? half_extents.x : -half_extents.x,
				   (p_normal.y > 0) ? half_extents.y : -half_extents.y,
				   (p_normal.z > 0) ? half_extents.z : -half_extents.z) +
			ofs;
}

template<arithmetic T>
Vector3_t<T> AABB_t<T>::get_endpoint(int p_point) const {
	switch (p_point) {
		default:
			CREATE_FAULT;
		case 0:
			return Vector3_t<T>(position.x, position.y, position.z);
		case 1:
			return Vector3_t<T>(position.x, position.y, position.z + size.z);
		case 2:
			return Vector3_t<T>(position.x, position.y + size.y, position.z);
		case 3:
			return Vector3_t<T>(position.x, position.y + size.y, position.z + size.z);
		case 4:
			return Vector3_t<T>(position.x + size.x, position.y, position.z);
		case 5:
			return Vector3_t<T>(position.x + size.x, position.y, position.z + size.z);
		case 6:
			return Vector3_t<T>(position.x + size.x, position.y + size.y, position.z);
		case 7:
			return Vector3_t<T>(position.x + size.x, position.y + size.y, position.z + size.z);
	}
}

template<arithmetic T>
bool AABB_t<T>::intersects_convex_shape(const Plane_t<T> *p_planes, int p_plane_count, const Vector3_t<T> *p_points, int p_point_count) const {
	Vector3_t<T> half_extents = size * static_cast<T>(0.5f);
	Vector3_t<T> ofs = position + half_extents;

	for (int i = 0; i < p_plane_count; i++) {
		const Plane_t<T> &p = p_planes[i];
		Vector3_t<T> point(
				(p.normal.x > 0) ? -half_extents.x : half_extents.x,
				(p.normal.y > 0) ? -half_extents.y : half_extents.y,
				(p.normal.z > 0) ? -half_extents.z : half_extents.z);
		point += ofs;
		if (p.is_point_over(point)) {
			return false;
		}
	}

	// Make sure all points in the shape aren't fully separated from the AABB_t on
	// each axis.
	int bad_point_counts_positive[3] = { 0 };
	int bad_point_counts_negative[3] = { 0 };

	for (int k = 0; k < 3; k++) {
		for (int i = 0; i < p_point_count; i++) {
			if (p_points[i][k] > ofs[k] + half_extents[k]) {
				bad_point_counts_positive[k]++;
			}
			if (p_points[i][k] < ofs[k] - half_extents[k]) {
				bad_point_counts_negative[k]++;
			}
		}

		if (bad_point_counts_negative[k] == p_point_count) {
			return false;
		}
		if (bad_point_counts_positive[k] == p_point_count) {
			return false;
		}
	}

	return true;
}

template<arithmetic T>
bool AABB_t<T>::inside_convex_shape(const Plane_t<T> *p_planes, int p_plane_count) const {
	Vector3_t<T> half_extents = size * static_cast<T>(0.5f);
	Vector3_t<T> ofs = position + half_extents;

	for (int i = 0; i < p_plane_count; i++) {
		const Plane_t<T> &p = p_planes[i];
		Vector3_t<T> point(
				(p.normal.x < 0) ? -half_extents.x : half_extents.x,
				(p.normal.y < 0) ? -half_extents.y : half_extents.y,
				(p.normal.z < 0) ? -half_extents.z : half_extents.z);
		point += ofs;
		if (p.is_point_over(point)) {
			return false;
		}
	}

	return true;
}

template<arithmetic T>
bool AABB_t<T>::has_point(const Vector3_t<T> & p_point) const {
	if (p_point.x < position.x) {
		return false;
	}
	if (p_point.y < position.y) {
		return false;
	}
	if (p_point.z < position.z) {
		return false;
	}
	if (p_point.x > position.x + size.x) {
		return false;
	}
	if (p_point.y > position.y + size.y) {
		return false;
	}
	if (p_point.z > position.z + size.z) {
		return false;
	}

	return true;
}

template<arithmetic T>
inline void AABB_t<T>::expand_to(const Vector3_t<T> & p_vector) {
	Vector3_t<T> begin = position;
	Vector3_t<T> end = position + size;

	if (p_vector.x < begin.x) {
		begin.x = p_vector.x;
	}
	if (p_vector.y < begin.y) {
		begin.y = p_vector.y;
	}
	if (p_vector.z < begin.z) {
		begin.z = p_vector.z;
	}

	if (p_vector.x > end.x) {
		end.x = p_vector.x;
	}
	if (p_vector.y > end.y) {
		end.y = p_vector.y;
	}
	if (p_vector.z > end.z) {
		end.z = p_vector.z;
	}

	position = begin;
	size = end - begin;
}

template<arithmetic T>
void AABB_t<T>::project_range_in_plane(const Plane_t<T> &p_plane, T &r_min, T &r_max) const {
	Vector3_t<T> half_extents(size.x * static_cast<T>(static_cast<T>(0.5f)), size.y * static_cast<T>(static_cast<T>(0.5f)), size.z * static_cast<T>(static_cast<T>(0.5f)));
	Vector3_t<T> center(position.x + half_extents.x, position.y + half_extents.y, position.z + half_extents.z);

	T length = p_plane.normal.abs().dot(half_extents);
	T distance = p_plane.distance_to(center);
	r_min = distance - length;
	r_max = distance + length;
}

template<arithmetic T>
inline T AABB_t<T>::get_longest_axis_size() const {
	T max_size = size.x;

	if (size.y > max_size) {
		max_size = size.y;
	}

	if (size.z > max_size) {
		max_size = size.z;
	}

	return max_size;
}

template<arithmetic T>
inline T AABB_t<T>::get_shortest_axis_size() const {
	T max_size = size.x;

	if (size.y < max_size) {
		max_size = size.y;
	}

	if (size.z < max_size) {
		max_size = size.z;
	}

	return max_size;
}

template<arithmetic T>
bool AABB_t<T>::smits_intersect_ray(const Vector3_t<T> & p_from, const Vector3_t<T> & p_dir, T p_t0, T p_t1) const {
	T divx = static_cast<T>(1) / p_dir.x;
	T divy = static_cast<T>(1) / p_dir.y;
	T divz = static_cast<T>(1) / p_dir.z;

	Vector3_t<T> upbound = position + size;
	T tmin, tmax, tymin, tymax, tzmin, tzmax;
	if (p_dir.x >= 0) {
		tmin = (position.x - p_from.x) * divx;
		tmax = (upbound.x - p_from.x) * divx;
	} else {
		tmin = (upbound.x - p_from.x) * divx;
		tmax = (position.x - p_from.x) * divx;
	}
	if (p_dir.y >= 0) {
		tymin = (position.y - p_from.y) * divy;
		tymax = (upbound.y - p_from.y) * divy;
	} else {
		tymin = (upbound.y - p_from.y) * divy;
		tymax = (position.y - p_from.y) * divy;
	}
	if ((tmin > tymax) or (tymin > tmax)) {
		return false;
	}
	if (tymin > tmin) {
		tmin = tymin;
	}
	if (tymax < tmax) {
		tmax = tymax;
	}
	if (p_dir.z >= 0) {
		tzmin = (position.z - p_from.z) * divz;
		tzmax = (upbound.z - p_from.z) * divz;
	} else {
		tzmin = (upbound.z - p_from.z) * divz;
		tzmax = (position.z - p_from.z) * divz;
	}
	if ((tmin > tzmax) or (tzmin > tmax)) {
		return false;
	}
	if (tzmin > tmin) {
		tmin = tzmin;
	}
	if (tzmax < tmax) {
		tmax = tzmax;
	}
	return ((tmin < p_t1) and (tmax > p_t0));
}

template<arithmetic T>
void AABB_t<T>::grow_by(T p_amount) {
	position.x -= p_amount;
	position.y -= p_amount;
	position.z -= p_amount;
	size.x += static_cast<T>(2) * p_amount;
	size.y += static_cast<T>(2) * p_amount;
	size.z += static_cast<T>(2) * p_amount;
}

template<arithmetic T>
void AABB_t<T>::quantize(T p_unit) {
	size += position;

	position.x -= fposmodp(position.x, p_unit);
	position.y -= fposmodp(position.y, p_unit);
	position.z -= fposmodp(position.z, p_unit);

	size.x -= fposmodp(size.x, p_unit);
	size.y -= fposmodp(size.y, p_unit);
	size.z -= fposmodp(size.z, p_unit);

	size.x += p_unit;
	size.y += p_unit;
	size.z += p_unit;

	size -= position;
}

template<arithmetic T>
AABB_t<T> AABB_t<T>::quantized(T p_unit) const {
	AABB_t<T> ret = *this;
	ret.quantize(p_unit);
	return ret;
}

template<arithmetic T>
__inline OutputStream & operator<<(OutputStream & os, const AABB_t<T> & aabb){
    os << "(";
	os << aabb.position << ',';
	os << aabb.size << ')';
	return os;
}

#include "aabb.tpp"

#endif // AABB_H
