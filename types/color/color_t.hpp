#ifndef __Color_t_HPP__

#define __Color_t_HPP__

#include <cstdint>

#include "../real.hpp"
#include "../src/errors/error_macros.h"
#include "string.h"


template <typename T>
struct Color_t {
private:
    T r;
    T g;
    T b;
    T a;
public:
	bool operator==(const Color_t &p_Color_t) const { return (r == p_Color_t.r && g == p_Color_t.g && b == p_Color_t.b && a == p_Color_t.a); }
	bool operator!=(const Color_t &p_Color_t) const { return (r != p_Color_t.r || g != p_Color_t.g || b != p_Color_t.b || a != p_Color_t.a); }

	uint32_t to_rgba32() const;
	uint32_t to_argb32() const;
	uint32_t to_abgr32() const;
	uint64_t to_rgba64() const;
	uint64_t to_argb64() const;
	uint64_t to_abgr64() const;
	T get_h() const;
	T get_s() const;
	T get_v() const;
	void set_hsv(T p_h, T p_s, T p_v, T p_alpha = T(1));

	__fast_inline T &operator[](const uint8_t idx) {
        static T default_value = T();
		return (idx < 4) ? *(&r + idx) : default_value;
	}
	__fast_inline const T &operator[](const uint8_t idx) const {
        static const T default_value = T();
		return (idx < 4) ? *(&r + idx) : default_value;
	}

	Color_t operator+(const Color_t &p_Color_t) const;
	void operator+=(const Color_t &p_Color_t);

	Color_t operator-() const;
	Color_t operator-(const Color_t &p_Color_t) const;
	void operator-=(const Color_t &p_Color_t);

	Color_t operator*(const Color_t &p_Color_t) const;
	Color_t operator*(const T &rvalue) const;
	void operator*=(const Color_t &p_Color_t);
	void operator*=(const T &rvalue);

	Color_t operator/(const Color_t &p_Color_t) const;
	Color_t operator/(const T &rvalue) const;
	void operator/=(const Color_t &p_Color_t);
	void operator/=(const T &rvalue);

	bool is_equal_approx(const Color_t &p_Color_t) const;

	void invert();
	void contrast();
	Color_t inverted() const;
	Color_t contrasted() const;

	__fast_inline Color_t linear_interpolate(const Color_t &p_to, T p_weight) const {
		Color_t res = *this;

		res.r += (p_weight * (p_to.r - r));
		res.g += (p_weight * (p_to.g - g));
		res.b += (p_weight * (p_to.b - b));
		res.a += (p_weight * (p_to.a - a));

		return res;
	}

	__fast_inline Color_t darkened(T p_amount) const {
		Color_t res = *this;
		res.r = res.r * (1.0f - p_amount);
		res.g = res.g * (1.0f - p_amount);
		res.b = res.b * (1.0f - p_amount);
		return res;
	}

	__fast_inline Color_t lightened(T p_amount) const {
		Color_t res = *this;
		res.r = res.r + (1.0f - res.r) * p_amount;
		res.g = res.g + (1.0f - res.g) * p_amount;
		res.b = res.b + (1.0f - res.b) * p_amount;
		return res;
	}

	__fast_inline Color_t blend(const Color_t &p_over) const {
		Color_t res;
		T sa = 1.0 - p_over.a;
		res.a = a * sa + p_over.a;
		if (res.a == 0) {
			return Color_t(T(), T(), T(), T());
		} else {
			res.r = (r * a * sa + p_over.r * p_over.a) / res.a;
			res.g = (g * a * sa + p_over.g * p_over.a) / res.a;
			res.b = (b * a * sa + p_over.b * p_over.a) / res.a;
		}
		return res;
	}

	__fast_inline Color_t to_linear() const {
		return Color_t(
				r < 0.04045 ? r * (1.0 / 12.92) : std::pow((r + 0.055) * (1.0 / (1 + 0.055)), T(2.4)),
				g < 0.04045 ? g * (1.0 / 12.92) : std::pow((g + 0.055) * (1.0 / (1 + 0.055)), T(2.4)),
				b < 0.04045 ? b * (1.0 / 12.92) : std::pow((b + 0.055) * (1.0 / (1 + 0.055)), T(2.4)),
				a);
	}
	__fast_inline Color_t to_srgb() const {
		return Color_t(
				r < 0.0031308 ? 12.92 * r : (1.0 + 0.055) * std::pow(r, T(1.0f / 2.4f)) - T(0.055),
				g < 0.0031308 ? 12.92 * g : (1.0 + 0.055) * std::pow(g, T(1.0f / 2.4f)) - T(0.055),
				b < 0.0031308 ? 12.92 * b : (1.0 + 0.055) * std::pow(b, T(1.0f / 2.4f)) - T(0.055), a);
	}

	static Color_t hex(uint32_t p_hex);
	static Color_t hex64(uint64_t p_hex);
	Color_t from_hsv(T p_h, T p_s, T p_v, T p_a) const;

	__fast_inline bool operator<(const Color_t &p_Color_t) const; //used in set keys

    
	__fast_inline Color_t() {
		r = 0;
		g = 0;
		b = 0;
		a = 1.0;
	}

    template <typename U>
	__fast_inline Color_t(U p_r, U p_g, U p_b, U p_a = U(1)) {
		r = static_cast<T>(p_r);
		g = static_cast<T>(p_g);
		b = static_cast<T>(p_b);
		a = static_cast<T>(p_a);
	}
};

#include "color_t.tpp"

#endif