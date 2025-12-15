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

#include "algebra/regions/plane.hpp"
#include "algebra/vectors/vec3.hpp"

/**
 * AABB (Axis Aligned Bounding Box)
 * This is implemented by a point (position) and the box size.
 */

namespace ymd{

template<arithmetic T>
struct AABB{
	Vec3<T> position;
	Vec3<T> size;

	T get_volume() const;
	__fast_inline bool has_volume() const {
		return size.x > 0 and size.y > 0 and size.z > 0;
	}

	__fast_inline bool has_surface() const {
		return size.x > 0 or size.y > 0 or size.z > 0;
	}

	const Vec3<T> & get_position() const { return position; }
	void set_position(const Vec3<T> & p_pos) { position = p_pos; }
	const Vec3<T> & get_size() const { return size; }
	void set_size(const Vec3<T> & p_size) { size = p_size; }

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
	__fast_inline bool smits_intersect_ray(const Vec3<T> & p_from, const Vec3<T> & p_dir, T p_t0, T p_t1) const;

	bool intersects_segment(const Vec3<T> & p_from, const Vec3<T> & p_to, Vec3<T> *r_intersection_point = nullptr, Vec3<T> *r_normal = nullptr) const;
	bool intersects_ray(const Vec3<T> & p_from, const Vec3<T> & p_dir) const {
		bool inside;
		return find_intersects_ray(p_from, p_dir, inside);
	}
	bool find_intersects_ray(const Vec3<T> & p_from, const Vec3<T> & p_dir, bool &r_inside, Vec3<T> *r_intersection_point = nullptr, Vec3<T> *r_normal = nullptr) const;

	__fast_inline bool intersects_convex_shape(const Plane<T> *p_planes, int p_plane_count, const Vec3<T> *p_points, int p_point_count) const;
	__fast_inline bool inside_convex_shape(const Plane<T> *p_planes, int p_plane_count) const;
	bool intersects_plane(const Plane<T> &p_plane) const;

	__fast_inline bool has_point(const Vec3<T> & p_point) const;
	__fast_inline Vec3<T> get_support(const Vec3<T> & p_normal) const;

	Vec3<T> get_longest_axis() const;
	int get_longest_axis_index() const;
	__fast_inline T get_longest_axis_size() const;

	Vec3<T> get_shortest_axis() const;
	int get_shortest_axis_index() const;
	__fast_inline T get_shortest_axis_size() const;

	AABB grow(T p_by) const;
	__fast_inline void grow_by(T p_amount);

	void get_edge(int p_edge, Vec3<T> &r_from, Vec3<T> &r_to) const;
	__fast_inline Vec3<T> get_endpoint(int p_point) const;

	AABB expand(const Vec3<T> & p_vector) const;

	template<typename... Args>
	requires std::conjunction_v<std::is_same_v<Vec3<T>, Args>...>
	AABB<T> expand(const Args&... points) const {
		AABB<T> aabb = *this;
		(aabb.expand_to(points), ...);
		return aabb;
	}

	__fast_inline void project_range_in_plane(const Plane<T> &p_plane, T &r_min, T &r_max) const;
	__fast_inline void expand_to(const Vec3<T> & p_vector); /** expand to contain a point if necessary */

	__fast_inline AABB abs() const {
		return AABB(position + size.minf(0), size.abs());
	}

	std::optional<Vec3<T>> intersects_segment_bind(const Vec3<T> & p_from, const Vec3<T> & p_to) const;
	std::optional<Vec3<T>> intersects_ray_bind(const Vec3<T> & p_from, const Vec3<T> & p_dir) const;

	__fast_inline void quantize(T p_unit);
	__fast_inline AABB quantized(T p_unit) const;

	__fast_inline void set_end(const Vec3<T> & p_end) {
		size = p_end - position;
	}

	__fast_inline Vec3<T> get_end() const {
		return position + size;
	}

	__fast_inline Vec3<T> get_center() const {
		return position + (size * T(static_cast<T>(0.5f)));
	}

	__fast_inline constexpr AABB() {}
	__fast_inline constexpr AABB(const Vec3<T> & p_pos, const Vec3<T> & p_size) :
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
	Vec3<T> src_min = position;
	Vec3<T> src_max = position + size;
	Vec3<T> dst_min = p_aabb.position;
	Vec3<T> dst_max = p_aabb.position + p_aabb.size;

	return (
			(src_min.x <= dst_min.x) and
			(src_max.x >= dst_max.x) and
			(src_min.y <= dst_min.y) and
			(src_max.y >= dst_max.y) and
			(src_min.z <= dst_min.z) and
			(src_max.z >= dst_max.z));
}

template<arithmetic T>
Vec3<T> AABB<T>::get_support(const Vec3<T> & p_normal) const {
	Vec3<T> half_extents = size * static_cast<T>(0.5f);
	Vec3<T> ofs = position + half_extents;

	return Vec3<T>(
				   (p_normal.x > 0) ? half_extents.x : -half_extents.x,
				   (p_normal.y > 0) ? half_extents.y : -half_extents.y,
				   (p_normal.z > 0) ? half_extents.z : -half_extents.z) +
			ofs;
}

template<arithmetic T>
Vec3<T> AABB<T>::get_endpoint(int p_point) const {
	switch (p_point) {
		default:
			HALT;
		case 0:
			return Vec3<T>(position.x, position.y, position.z);
		case 1:
			return Vec3<T>(position.x, position.y, position.z + size.z);
		case 2:
			return Vec3<T>(position.x, position.y + size.y, position.z);
		case 3:
			return Vec3<T>(position.x, position.y + size.y, position.z + size.z);
		case 4:
			return Vec3<T>(position.x + size.x, position.y, position.z);
		case 5:
			return Vec3<T>(position.x + size.x, position.y, position.z + size.z);
		case 6:
			return Vec3<T>(position.x + size.x, position.y + size.y, position.z);
		case 7:
			return Vec3<T>(position.x + size.x, position.y + size.y, position.z + size.z);
	}
}

template<arithmetic T>
bool AABB<T>::intersects_convex_shape(const Plane<T> *p_planes, int p_plane_count, const Vec3<T> *p_points, int p_point_count) const {
	Vec3<T> half_extents = size * static_cast<T>(0.5f);
	Vec3<T> ofs = position + half_extents;

	for (int i = 0; i < p_plane_count; i++) {
		const Plane<T> &p = p_planes[i];
		Vec3<T> point(
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
	Vec3<T> half_extents = size * static_cast<T>(0.5f);
	Vec3<T> ofs = position + half_extents;

	for (int i = 0; i < p_plane_count; i++) {
		const Plane<T> &p = p_planes[i];
		Vec3<T> point(
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
bool AABB<T>::has_point(const Vec3<T> & p_point) const {
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
inline void AABB<T>::expand_to(const Vec3<T> & p_vector) {
	Vec3<T> begin = position;
	Vec3<T> end = position + size;

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
	Vec3<T> half_extents(size.x * static_cast<T>(static_cast<T>(0.5f)), size.y * static_cast<T>(static_cast<T>(0.5f)), size.z * static_cast<T>(static_cast<T>(0.5f)));
	Vec3<T> center(position.x + half_extents.x, position.y + half_extents.y, position.z + half_extents.z);

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
bool AABB<T>::smits_intersect_ray(const Vec3<T> & p_from, const Vec3<T> & p_dir, T p_t0, T p_t1) const {
	T divx = static_cast<T>(1) / p_dir.x;
	T divy = static_cast<T>(1) / p_dir.y;
	T divz = static_cast<T>(1) / p_dir.z;

	Vec3<T> upbound = position + size;
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




namespace ymd{

template<arithmetic T>
T AABB<T>::get_volume() const {
	return size.x * size.y * size.z;
}

template<arithmetic T>
bool AABB<T>::operator==(const AABB<T> & p_rval) const {
	return ((position == p_rval.position) && (size == p_rval.size));
}

template<arithmetic T>
bool AABB<T>::operator!=(const AABB<T> & p_rval) const {
	return ((position != p_rval.position) || (size != p_rval.size));
}

template<arithmetic T>
void AABB<T>::merge_with(const AABB<T> & p_aabb) {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0 || size.y < 0 || size.z < 0 || p_aabb.size.x < 0 || p_aabb.size.y < 0 || p_aabb.size.z < 0)) {
		ERR_PRINT("AABB size is negative, this is not supported. Use AABB.abs() to get an AABB with a positive size.");
	}
#endif
	Vec3<T> beg_1, beg_2;
	Vec3<T> end_1, end_2;
	Vec3<T> min, max;

	beg_1 = position;
	beg_2 = p_aabb.position;
	end_1 = size + beg_1;
	end_2 = p_aabb.size + beg_2;

	min.x = (beg_1.x < beg_2.x) ? beg_1.x : beg_2.x;
	min.y = (beg_1.y < beg_2.y) ? beg_1.y : beg_2.y;
	min.z = (beg_1.z < beg_2.z) ? beg_1.z : beg_2.z;

	max.x = (end_1.x > end_2.x) ? end_1.x : end_2.x;
	max.y = (end_1.y > end_2.y) ? end_1.y : end_2.y;
	max.z = (end_1.z > end_2.z) ? end_1.z : end_2.z;

	position = min;
	size = max - min;
}

template<arithmetic T>
bool AABB<T>::is_equal_approx(const AABB<T> & p_aabb) const {
	return position.is_equal_approx(p_aabb.position) && size.is_equal_approx(p_aabb.size);
}

template<arithmetic T>
bool AABB<T>::is_finite() const {
	return position.is_finite() && size.is_finite();
}

template<arithmetic T>
AABB<T> AABB<T>::intersection(const AABB<T> & p_aabb) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0 || size.y < 0 || size.z < 0 || p_aabb.size.x < 0 || p_aabb.size.y < 0 || p_aabb.size.z < 0)) {
		ERR_PRINT("AABB size is negative, this is not supported. Use AABB.abs() to get an AABB with a positive size.");
	}
#endif
	Vec3<T> src_min = position;
	Vec3<T> src_max = position + size;
	Vec3<T> dst_min = p_aabb.position;
	Vec3<T> dst_max = p_aabb.position + p_aabb.size;

	Vec3<T> min, max;

	if (src_min.x > dst_max.x || src_max.x < dst_min.x) {
		return AABB<T>();
	} else {
		min.x = (src_min.x > dst_min.x) ? src_min.x : dst_min.x;
		max.x = (src_max.x < dst_max.x) ? src_max.x : dst_max.x;
	}

	if (src_min.y > dst_max.y || src_max.y < dst_min.y) {
		return AABB<T>();
	} else {
		min.y = (src_min.y > dst_min.y) ? src_min.y : dst_min.y;
		max.y = (src_max.y < dst_max.y) ? src_max.y : dst_max.y;
	}

	if (src_min.z > dst_max.z || src_max.z < dst_min.z) {
		return AABB<T>();
	} else {
		min.z = (src_min.z > dst_min.z) ? src_min.z : dst_min.z;
		max.z = (src_max.z < dst_max.z) ? src_max.z : dst_max.z;
	}

	return AABB(min, max - min);
}

// Note that this routine returns the BACKTRACKED (i.e. behind the ray origin)
// intersection point + normal if INSIDE the AABB.
// The caller can therefore decide when INSIDE whether to use the
// backtracked intersection, or use p_from as the intersection, and
// carry on progressing without e.g. reflecting against the normal.
template<arithmetic T>
bool AABB<T>::find_intersects_ray(const Vec3<T> & p_from, const Vec3<T> & p_dir, bool &r_inside, Vec3<T> *r_intersection_point, Vec3<T> *r_normal) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0 || size.y < 0 || size.z < 0)) {
		ERR_PRINT("AABB size is negative, this is not supported. Use AABB.abs() to get an AABB with a positive size.");
	}
#endif
	Vec3<T> end = position + size;
	T tmin = -1e20;
	T tmax = 1e20;
	int axis = 0;

	// Make sure r_inside is always initialized,
	// to prevent reading uninitialized data in the client code.
	r_inside = false;

	for (int i = 0; i < 3; i++) {
		if (p_dir[i] == 0) {
			if ((p_from[i] < position[i]) || (p_from[i] > end[i])) {
				return false;
			}
		} else { // ray not parallel to planes in this direction
			T t1 = (position[i] - p_from[i]) / p_dir[i];
			T t2 = (end[i] - p_from[i]) / p_dir[i];

			if (t1 > t2) {
				SWAP(t1, t2);
			}
			if (t1 >= tmin) {
				tmin = t1;
				axis = i;
			}
			if (t2 < tmax) {
				if (t2 < 0) {
					return false;
				}
				tmax = t2;
			}
			if (tmin > tmax) {
				return false;
			}
		}
	}

	// Did the ray start from inside the box?
	// In which case the intersection returned is the point of entry
	// (behind the ray start) or the calling routine can use the ray origin as intersection point.
	r_inside = tmin < 0;

	if (r_intersection_point) {
		*r_intersection_point = p_from + p_dir * tmin;

		// Prevent float error by making sure the point is exactly
		// on the AABB border on the relevant axis.
		r_intersection_point->coord[axis] = (p_dir[axis] >= 0) ? position.coord[axis] : end.coord[axis];
	}
	if (r_normal) {
		*r_normal = Vec3<T>();
		(*r_normal)[axis] = (p_dir[axis] >= 0) ? -1 : 1;
	}

	return true;
}

template<arithmetic T>
bool AABB<T>::intersects_segment(const Vec3<T> & p_from, const Vec3<T> & p_to, Vec3<T> *r_intersection_point, Vec3<T> *r_normal) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0 || size.y < 0 || size.z < 0)) {
		ERR_PRINT("AABB size is negative, this is not supported. Use AABB.abs() to get an AABB with a positive size.");
	}
#endif
	T min = 0, max = 1;
	int axis = 0;
	T sign = 0;

	for (int i = 0; i < 3; i++) {
		T seg_from = p_from[i];
		T seg_to = p_to[i];
		T box_begin = position[i];
		T box_end = box_begin + size[i];
		T cmin, cmax;
		T csign;

		if (seg_from < seg_to) {
			if (seg_from > box_end || seg_to < box_begin) {
				return false;
			}
			T length = seg_to - seg_from;
			cmin = (seg_from < box_begin) ? ((box_begin - seg_from) / length) : 0;
			cmax = (seg_to > box_end) ? ((box_end - seg_from) / length) : 1;
			csign = -1.0;

		} else {
			if (seg_to > box_end || seg_from < box_begin) {
				return false;
			}
			T length = seg_to - seg_from;
			cmin = (seg_from > box_end) ? (box_end - seg_from) / length : 0;
			cmax = (seg_to < box_begin) ? (box_begin - seg_from) / length : 1;
			csign = 1.0;
		}

		if (cmin > min) {
			min = cmin;
			axis = i;
			sign = csign;
		}
		if (cmax < max) {
			max = cmax;
		}
		if (max < min) {
			return false;
		}
	}

	Vec3<T> rel = p_to - p_from;

	if (r_normal) {
		Vec3<T> normal;
		normal[axis] = sign;
		*r_normal = normal;
	}

	if (r_intersection_point) {
		*r_intersection_point = p_from + rel * min;
	}

	return true;
}

template<arithmetic T>
bool AABB<T>::intersects_plane(const Plane<T> &p_plane) const {
	Vec3<T> points[8] = {
		Vec3<T>(position.x, position.y, position.z),
		Vec3<T>(position.x, position.y, position.z + size.z),
		Vec3<T>(position.x, position.y + size.y, position.z),
		Vec3<T>(position.x, position.y + size.y, position.z + size.z),
		Vec3<T>(position.x + size.x, position.y, position.z),
		Vec3<T>(position.x + size.x, position.y, position.z + size.z),
		Vec3<T>(position.x + size.x, position.y + size.y, position.z),
		Vec3<T>(position.x + size.x, position.y + size.y, position.z + size.z),
	};

	bool over = false;
	bool under = false;

	for (int i = 0; i < 8; i++) {
		if (p_plane.distance_to(points[i]) > 0) {
			over = true;
		} else {
			under = true;
		}
	}

	return under && over;
}

template<arithmetic T>
Vec3<T> AABB<T>::get_longest_axis() const {
	Vec3<T> axis(1, 0, 0);
	T max_size = size.x;

	if (size.y > max_size) {
		axis = Vec3<T>(0, 1, 0);
		max_size = size.y;
	}

	if (size.z > max_size) {
		axis = Vec3<T>(0, 0, 1);
	}

	return axis;
}

template<arithmetic T>
int AABB<T>::get_longest_axis_index() const {
	int axis = 0;
	T max_size = size.x;

	if (size.y > max_size) {
		axis = 1;
		max_size = size.y;
	}

	if (size.z > max_size) {
		axis = 2;
	}

	return axis;
}

template<arithmetic T>
Vec3<T> AABB<T>::get_shortest_axis() const {
	Vec3<T> axis(1, 0, 0);
	T min_size = size.x;

	if (size.y < min_size) {
		axis = Vec3<T>(0, 1, 0);
		min_size = size.y;
	}

	if (size.z < min_size) {
		axis = Vec3<T>(0, 0, 1);
	}

	return axis;
}

template<arithmetic T>
int AABB<T>::get_shortest_axis_index() const {
	int axis = 0;
	T min_size = size.x;

	if (size.y < min_size) {
		axis = 1;
		min_size = size.y;
	}

	if (size.z < min_size) {
		axis = 2;
	}

	return axis;
}

template<arithmetic T>
AABB<T> AABB<T>::merge(const AABB<T> & p_with) const {
	AABB<T> aabb = *this;
	aabb.merge_with(p_with);
	return aabb;
}

template<arithmetic T>
AABB<T> AABB<T>::expand(const Vec3<T> & p_vector) const {
	AABB<T> aabb = *this;
	aabb.expand_to(p_vector);
	return aabb;
}

template<arithmetic T>
AABB<T> AABB<T>::grow(T p_by) const {
	AABB<T> aabb = *this;
	aabb.grow_by(p_by);
	return aabb;
}

template<arithmetic T>
void AABB<T>::get_edge(int p_edge, Vec3<T> &r_from, Vec3<T> &r_to) const {
	switch (p_edge) {
		default:
		    HALT;
		case 0: {
			r_from = Vec3(position.x + size.x, position.y, position.z);
			r_to = Vec3(position.x, position.y, position.z);
		} break;
		case 1: {
			r_from = Vec3(position.x + size.x, position.y, position.z + size.z);
			r_to = Vec3(position.x + size.x, position.y, position.z);
		} break;
		case 2: {
			r_from = Vec3(position.x, position.y, position.z + size.z);
			r_to = Vec3(position.x + size.x, position.y, position.z + size.z);

		} break;
		case 3: {
			r_from = Vec3(position.x, position.y, position.z);
			r_to = Vec3(position.x, position.y, position.z + size.z);

		} break;
		case 4: {
			r_from = Vec3(position.x, position.y + size.y, position.z);
			r_to = Vec3(position.x + size.x, position.y + size.y, position.z);
		} break;
		case 5: {
			r_from = Vec3(position.x + size.x, position.y + size.y, position.z);
			r_to = Vec3(position.x + size.x, position.y + size.y, position.z + size.z);
		} break;
		case 6: {
			r_from = Vec3(position.x + size.x, position.y + size.y, position.z + size.z);
			r_to = Vec3(position.x, position.y + size.y, position.z + size.z);

		} break;
		case 7: {
			r_from = Vec3(position.x, position.y + size.y, position.z + size.z);
			r_to = Vec3(position.x, position.y + size.y, position.z);

		} break;
		case 8: {
			r_from = Vec3(position.x, position.y, position.z + size.z);
			r_to = Vec3(position.x, position.y + size.y, position.z + size.z);

		} break;
		case 9: {
			r_from = Vec3(position.x, position.y, position.z);
			r_to = Vec3(position.x, position.y + size.y, position.z);

		} break;
		case 10: {
			r_from = Vec3(position.x + size.x, position.y, position.z);
			r_to = Vec3(position.x + size.x, position.y + size.y, position.z);

		} break;
		case 11: {
			r_from = Vec3(position.x + size.x, position.y, position.z + size.z);
			r_to = Vec3(position.x + size.x, position.y + size.y, position.z + size.z);

		} break;
	}
}

template<arithmetic T>
std::optional<Vec3<T>> AABB<T>::intersects_segment_bind(const Vec3<T> & p_from, const Vec3<T> & p_to) const {
	Vec3<T> inters;
	if (intersects_segment(p_from, p_to, &inters)) {
		return inters;
	}
	return std::nullopt;
}

template<arithmetic T>
std::optional<Vec3<T>> AABB<T>::intersects_ray_bind(const Vec3<T> & p_from, const Vec3<T> & p_dir) const {
	Vec3<T> inters;
	bool inside = false;

	if (find_intersects_ray(p_from, p_dir, inside, &inters)) {
		// When inside the intersection point may be BEHIND the ray,
		// so for general use we return the ray origin.
		if (inside) {
			return p_from;
		}

		return inters;
	}
	return std::nullopt;
}

}