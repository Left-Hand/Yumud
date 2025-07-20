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

#include "types/regions/plane/plane.hpp"
#include "types/vectors/vector3/vector3.hpp"

/**
 * AABB (Axis Aligned Bounding Box)
 * This is implemented by a point (position) and the box size.
 */

namespace ymd{

template<arithmetic T>
struct AABB{
	Vector3<T> position;
	Vector3<T> size;

	T get_volume() const;
	__fast_inline bool has_volume() const {
		return size.x > 0 and size.y > 0 and size.z > 0;
	}

	__fast_inline bool has_surface() const {
		return size.x > 0 or size.y > 0 or size.z > 0;
	}

	const Vector3<T> & get_position() const { return position; }
	void set_position(const Vector3<T> & p_pos) { position = p_pos; }
	const Vector3<T> & get_size() const { return size; }
	void set_size(const Vector3<T> & p_size) { size = p_size; }

	bool operator==(const AABB<T> &p_rval) const;
	bool operator!=(const AABB<T> &p_rval) const;

	bool is_equal_approx(const AABB<T> &p_aabb) const;
	bool is_finite() const;
	__fast_inline bool intersects(const AABB<T> &p_aabb) const; /// Both AABBs overlap
	__fast_inline bool intersects_inclusive(const AABB<T> &p_aabb) const; /// Both AABBs (or their faces) overlap
	__fast_inline bool encloses(const AABB<T> &p_aabb) const; /// p_aabb is completely inside this

	AABB merge(const AABB<T> &p_with) const;
	void merge_with(const AABB<T> &p_aabb); ///merge with another AABB
	AABB intersection(const AABB<T> &p_aabb) const; ///get box where two intersect, empty if no intersection occurs
	__fast_inline bool smits_intersect_ray(const Vector3<T> & p_from, const Vector3<T> & p_dir, T p_t0, T p_t1) const;

	bool intersects_segment(const Vector3<T> & p_from, const Vector3<T> & p_to, Vector3<T> *r_intersection_point = nullptr, Vector3<T> *r_normal = nullptr) const;
	bool intersects_ray(const Vector3<T> & p_from, const Vector3<T> & p_dir) const {
		bool inside;
		return find_intersects_ray(p_from, p_dir, inside);
	}
	bool find_intersects_ray(const Vector3<T> & p_from, const Vector3<T> & p_dir, bool &r_inside, Vector3<T> *r_intersection_point = nullptr, Vector3<T> *r_normal = nullptr) const;

	__fast_inline bool intersects_convex_shape(const Plane<T> *p_planes, int p_plane_count, const Vector3<T> *p_points, int p_point_count) const;
	__fast_inline bool inside_convex_shape(const Plane<T> *p_planes, int p_plane_count) const;
	bool intersects_plane(const Plane<T> &p_plane) const;

	__fast_inline bool has_point(const Vector3<T> & p_point) const;
	__fast_inline Vector3<T> get_support(const Vector3<T> & p_normal) const;

	Vector3<T> get_longest_axis() const;
	int get_longest_axis_index() const;
	__fast_inline T get_longest_axis_size() const;

	Vector3<T> get_shortest_axis() const;
	int get_shortest_axis_index() const;
	__fast_inline T get_shortest_axis_size() const;

	AABB grow(T p_by) const;
	__fast_inline void grow_by(T p_amount);

	void get_edge(int p_edge, Vector3<T> &r_from, Vector3<T> &r_to) const;
	__fast_inline Vector3<T> get_endpoint(int p_point) const;

	AABB expand(const Vector3<T> & p_vector) const;

	template<typename... Args>
	requires std::conjunction_v<std::is_same_v<Vector3<T>, Args>...>
	AABB<T> expand(const Args&... points) const {
		AABB<T> aabb = *this;
		(aabb.expand_to(points), ...);
		return aabb;
	}

	__fast_inline void project_range_in_plane(const Plane<T> &p_plane, T &r_min, T &r_max) const;
	__fast_inline void expand_to(const Vector3<T> & p_vector); /** expand to contain a point if necessary */

	__fast_inline AABB abs() const {
		return AABB(position + size.minf(0), size.abs());
	}

	std::optional<Vector3<T>> intersects_segment_bind(const Vector3<T> & p_from, const Vector3<T> & p_to) const;
	std::optional<Vector3<T>> intersects_ray_bind(const Vector3<T> & p_from, const Vector3<T> & p_dir) const;

	__fast_inline void quantize(T p_unit);
	__fast_inline AABB quantized(T p_unit) const;

	__fast_inline void set_end(const Vector3<T> & p_end) {
		size = p_end - position;
	}

	__fast_inline Vector3<T> get_end() const {
		return position + size;
	}

	__fast_inline Vector3<T> get_center() const {
		return position + (size * T(static_cast<T>(0.5f)));
	}

	__fast_inline constexpr AABB() {}
	__fast_inline constexpr AABB(const Vector3<T> & p_pos, const Vector3<T> & p_size) :
			position(p_pos),
			size(p_size) {
	}
};

template<arithmetic T>
inline bool AABB<T>::intersects(const AABB<T> &p_aabb) const {
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
inline bool AABB<T>::intersects_inclusive(const AABB<T> &p_aabb) const {
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
inline bool AABB<T>::encloses(const AABB<T> &p_aabb) const {
	Vector3<T> src_min = position;
	Vector3<T> src_max = position + size;
	Vector3<T> dst_min = p_aabb.position;
	Vector3<T> dst_max = p_aabb.position + p_aabb.size;

	return (
			(src_min.x <= dst_min.x) and
			(src_max.x >= dst_max.x) and
			(src_min.y <= dst_min.y) and
			(src_max.y >= dst_max.y) and
			(src_min.z <= dst_min.z) and
			(src_max.z >= dst_max.z));
}

template<arithmetic T>
Vector3<T> AABB<T>::get_support(const Vector3<T> & p_normal) const {
	Vector3<T> half_extents = size * static_cast<T>(0.5f);
	Vector3<T> ofs = position + half_extents;

	return Vector3<T>(
				   (p_normal.x > 0) ? half_extents.x : -half_extents.x,
				   (p_normal.y > 0) ? half_extents.y : -half_extents.y,
				   (p_normal.z > 0) ? half_extents.z : -half_extents.z) +
			ofs;
}

template<arithmetic T>
Vector3<T> AABB<T>::get_endpoint(int p_point) const {
	switch (p_point) {
		default:
			HALT;
		case 0:
			return Vector3<T>(position.x, position.y, position.z);
		case 1:
			return Vector3<T>(position.x, position.y, position.z + size.z);
		case 2:
			return Vector3<T>(position.x, position.y + size.y, position.z);
		case 3:
			return Vector3<T>(position.x, position.y + size.y, position.z + size.z);
		case 4:
			return Vector3<T>(position.x + size.x, position.y, position.z);
		case 5:
			return Vector3<T>(position.x + size.x, position.y, position.z + size.z);
		case 6:
			return Vector3<T>(position.x + size.x, position.y + size.y, position.z);
		case 7:
			return Vector3<T>(position.x + size.x, position.y + size.y, position.z + size.z);
	}
}

template<arithmetic T>
bool AABB<T>::intersects_convex_shape(const Plane<T> *p_planes, int p_plane_count, const Vector3<T> *p_points, int p_point_count) const {
	Vector3<T> half_extents = size * static_cast<T>(0.5f);
	Vector3<T> ofs = position + half_extents;

	for (int i = 0; i < p_plane_count; i++) {
		const Plane<T> &p = p_planes[i];
		Vector3<T> point(
				(p.normal.x > 0) ? -half_extents.x : half_extents.x,
				(p.normal.y > 0) ? -half_extents.y : half_extents.y,
				(p.normal.z > 0) ? -half_extents.z : half_extents.z);
		point += ofs;
		if (p.is_point_over(point)) {
			return false;
		}
	}

	// Make sure all points in the shape aren't fully separated from the AABB on
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
bool AABB<T>::inside_convex_shape(const Plane<T> *p_planes, int p_plane_count) const {
	Vector3<T> half_extents = size * static_cast<T>(0.5f);
	Vector3<T> ofs = position + half_extents;

	for (int i = 0; i < p_plane_count; i++) {
		const Plane<T> &p = p_planes[i];
		Vector3<T> point(
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
bool AABB<T>::has_point(const Vector3<T> & p_point) const {
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
inline void AABB<T>::expand_to(const Vector3<T> & p_vector) {
	Vector3<T> begin = position;
	Vector3<T> end = position + size;

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
void AABB<T>::project_range_in_plane(const Plane<T> &p_plane, T &r_min, T &r_max) const {
	Vector3<T> half_extents(size.x * static_cast<T>(static_cast<T>(0.5f)), size.y * static_cast<T>(static_cast<T>(0.5f)), size.z * static_cast<T>(static_cast<T>(0.5f)));
	Vector3<T> center(position.x + half_extents.x, position.y + half_extents.y, position.z + half_extents.z);

	T length = p_plane.normal.abs().dot(half_extents);
	T distance = p_plane.distance_to(center);
	r_min = distance - length;
	r_max = distance + length;
}

template<arithmetic T>
inline T AABB<T>::get_longest_axis_size() const {
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
inline T AABB<T>::get_shortest_axis_size() const {
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
bool AABB<T>::smits_intersect_ray(const Vector3<T> & p_from, const Vector3<T> & p_dir, T p_t0, T p_t1) const {
	T divx = static_cast<T>(1) / p_dir.x;
	T divy = static_cast<T>(1) / p_dir.y;
	T divz = static_cast<T>(1) / p_dir.z;

	Vector3<T> upbound = position + size;
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
void AABB<T>::grow_by(T p_amount) {
	position.x -= p_amount;
	position.y -= p_amount;
	position.z -= p_amount;
	size.x += static_cast<T>(2) * p_amount;
	size.y += static_cast<T>(2) * p_amount;
	size.z += static_cast<T>(2) * p_amount;
}

template<arithmetic T>
void AABB<T>::quantize(T p_unit) {
	size += position;

	position.x -= fposmod(position.x, p_unit);
	position.y -= fposmod(position.y, p_unit);
	position.z -= fposmod(position.z, p_unit);

	size.x -= fposmod(size.x, p_unit);
	size.y -= fposmod(size.y, p_unit);
	size.z -= fposmod(size.z, p_unit);

	size.x += p_unit;
	size.y += p_unit;
	size.z += p_unit;

	size -= position;
}

template<arithmetic T>
AABB<T> AABB<T>::quantized(T p_unit) const {
	AABB<T> ret = *this;
	ret.quantize(p_unit);
	return ret;
}

template<arithmetic T>
__inline OutputStream & operator<<(OutputStream & os, const AABB<T> & aabb){
	const auto splt = os.splitter();
    os << "(";
	os << aabb.position << splt;
	os << aabb.size << ')';
	return os;
}


}


#include "aabb.tpp"