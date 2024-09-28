

template<arithmetic T>
void Plane_t<T>::set_normal(const Vector3_t<auto> &p_normal) {
	normal = p_normal;
}


template<arithmetic T>
void Plane_t<T>::normalize() {
	T l = normal.length();
	if (l == 0) {
		*this = Plane_t<T>(0, 0, 0, 0);
		return;
	}
	normal /= l;
	d /= l;
}


template<arithmetic T>
Plane_t<T> Plane_t<T>::normalized() const {
	Plane_t<T> p = *this;
	p.normalize();
	return p;
}


template<arithmetic T>
Vector3_t<T> Plane_t<T>::get_any_perpendicular_normal() const {
	scexpr auto p1 = Vector3_t<T>(1, 0, 0);
	scexpr auto p2 = Vector3_t<T>(0, 1, 0);
	Vector3_t<T> p;

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
bool Plane_t<T>::intersect_3(const Plane_t<T> &p_plane1, const Plane_t<T> &p_plane2, Vector3_t<T> & r_result) const {
	const Plane_t<T> &p_plane0 = *this;
	Vector3_t<T> normal0 = p_plane0.normal;
	Vector3_t<T> normal1 = p_plane1.normal;
	Vector3_t<T> normal2 = p_plane2.normal;

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
bool Plane_t<T>::intersects_ray(const Vector3_t<T> &p_from, const Vector3_t<T> &p_dir, Vector3_t<T> & p_intersection) const {
	Vector3_t<T> segment = p_dir;
	T den = normal.dot(segment);

	if (::is_equal_approx(0, den)) {
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
bool Plane_t<T>::intersects_segment(const Vector3_t<T> &p_begin, const Vector3_t<T> &p_end, Vector3_t<T> & p_intersection) const {
	Vector3_t<T> segment = p_begin - p_end;
	T den = normal.dot(segment);
	if (::is_equal_approx(0, den)) {
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
std::optional<Vector3_t<T>> Plane_t<T>::intersect_3(const Plane_t<T> &p_plane1, const Plane_t<T> &p_plane2) const {
	Vector3_t<T> inters;
	if (intersect_3(p_plane1, p_plane2, inters)) {
		return inters;
	} else {
		return std::nullopt;
	}
}


template<arithmetic T>
std::optional<Vector3_t<T>> Plane_t<T>::intersects_ray(const Vector3_t<T> &p_from, const Vector3_t<T> &p_dir) const {
	Vector3_t<T> inters;
	if (intersects_ray(p_from, p_dir, inters)) {
		return inters;
	} else {
		return std::nullopt;
	}
}


template<arithmetic T>
std::optional<Vector3_t<T>> Plane_t<T>::intersects_segment(const Vector3_t<T> &p_begin, const Vector3_t<T> &p_end) const {
	Vector3_t<T> inters;
	if (intersects_segment(p_begin, p_end, inters)) {
		return inters;
	} else {
		return std::nullopt;
	}
}

/* misc */

template<arithmetic T>
bool Plane_t<T>::is_equal_approx_any_side(const Plane_t<T> &p_plane) const {
	return (normal.is_equal_approx(p_plane.normal) && ::is_equal_approx(d, p_plane.d)) || (normal.is_equal_approx(-p_plane.normal) && ::is_equal_approx(d, -p_plane.d));
}


template<arithmetic T>
bool Plane_t<T>::is_equal_approx(const Plane_t<T> &p_plane) const {
	return normal.is_equal_approx(p_plane.normal) && ::is_equal_approx(d, p_plane.d);
}


template<arithmetic T>
bool Plane_t<T>::is_finite() const {
	return normal.is_finite() && is_finite(d);
}
