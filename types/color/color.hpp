#ifndef __COLOR_HPP__

#define __COLOR_HPP__

#include <cstdint>

#include "defines/comm_inc.h"
#include "../real.hpp"
#include "../src/errors/error_macros.h"
#include "string.h"


struct Color {

    real_t r;
    real_t g;
    real_t b;
    real_t a;

	bool operator==(const Color &p_color) const { return (r == p_color.r && g == p_color.g && b == p_color.b && a == p_color.a); }
	bool operator!=(const Color &p_color) const { return (r != p_color.r || g != p_color.g || b != p_color.b || a != p_color.a); }

	uint32_t to_rgba32() const;
	uint32_t to_argb32() const;
	uint32_t to_abgr32() const;
	uint64_t to_rgba64() const;
	uint64_t to_argb64() const;
	uint64_t to_abgr64() const;
	real_t get_h() const;
	real_t get_s() const;
	real_t get_v() const;
	void set_hsv(real_t p_h, real_t p_s, real_t p_v, real_t p_alpha = real_t(1));

	_FORCE_INLINE_ real_t &operator[](const uint8_t idx) {
        static real_t default_value = real_t();
		return (idx < 4) ? *(&r + idx) : default_value;
	}
	_FORCE_INLINE_ const real_t &operator[](const uint8_t idx) const {
        static const real_t default_value = real_t();
		return (idx < 4) ? *(&r + idx) : default_value;
	}

	Color operator+(const Color &p_color) const;
	void operator+=(const Color &p_color);

	Color operator-() const;
	Color operator-(const Color &p_color) const;
	void operator-=(const Color &p_color);

	Color operator*(const Color &p_color) const;
	Color operator*(const real_t &rvalue) const;
	void operator*=(const Color &p_color);
	void operator*=(const real_t &rvalue);

	Color operator/(const Color &p_color) const;
	Color operator/(const real_t &rvalue) const;
	void operator/=(const Color &p_color);
	void operator/=(const real_t &rvalue);

	bool is_equal_approx(const Color &p_color) const;

	void invert();
	void contrast();
	Color inverted() const;
	Color contrasted() const;

	_FORCE_INLINE_ Color linear_interpolate(const Color &p_to, real_t p_weight) const {
		Color res = *this;

		res.r += (p_weight * (p_to.r - r));
		res.g += (p_weight * (p_to.g - g));
		res.b += (p_weight * (p_to.b - b));
		res.a += (p_weight * (p_to.a - a));

		return res;
	}

	_FORCE_INLINE_ Color darkened(real_t p_amount) const {
		Color res = *this;
		res.r = res.r * (1.0f - p_amount);
		res.g = res.g * (1.0f - p_amount);
		res.b = res.b * (1.0f - p_amount);
		return res;
	}

	_FORCE_INLINE_ Color lightened(real_t p_amount) const {
		Color res = *this;
		res.r = res.r + (1.0f - res.r) * p_amount;
		res.g = res.g + (1.0f - res.g) * p_amount;
		res.b = res.b + (1.0f - res.b) * p_amount;
		return res;
	}

	_FORCE_INLINE_ Color blend(const Color &p_over) const {
		Color res;
		real_t sa = 1.0 - p_over.a;
		res.a = a * sa + p_over.a;
		if (res.a == 0) {
			return Color(real_t(), real_t(), real_t(), real_t());
		} else {
			res.r = (r * a * sa + p_over.r * p_over.a) / res.a;
			res.g = (g * a * sa + p_over.g * p_over.a) / res.a;
			res.b = (b * a * sa + p_over.b * p_over.a) / res.a;
		}
		return res;
	}

	_FORCE_INLINE_ Color to_linear() const {
		return Color(
				r < 0.04045 ? r * (1.0 / 12.92) : std::pow((r + 0.055) * (1.0 / (1 + 0.055)), real_t(2.4)),
				g < 0.04045 ? g * (1.0 / 12.92) : std::pow((g + 0.055) * (1.0 / (1 + 0.055)), real_t(2.4)),
				b < 0.04045 ? b * (1.0 / 12.92) : std::pow((b + 0.055) * (1.0 / (1 + 0.055)), real_t(2.4)),
				a);
	}
	_FORCE_INLINE_ Color to_srgb() const {
		return Color(
				r < 0.0031308 ? 12.92 * r : (1.0 + 0.055) * std::pow(r, real_t(1.0f / 2.4f)) - real_t(0.055),
				g < 0.0031308 ? 12.92 * g : (1.0 + 0.055) * std::pow(g, real_t(1.0f / 2.4f)) - real_t(0.055),
				b < 0.0031308 ? 12.92 * b : (1.0 + 0.055) * std::pow(b, real_t(1.0f / 2.4f)) - real_t(0.055), a);
	}

	static Color hex(uint32_t p_hex);
	static Color hex64(uint64_t p_hex);
	Color from_hsv(real_t p_h, real_t p_s, real_t p_v, real_t p_a) const;

	_FORCE_INLINE_ bool operator<(const Color &p_color) const; //used in set keys

	/**
	 * No construct parameters, r=0, g=0, b=0. a=255
	 */
	_FORCE_INLINE_ Color() {
		r = 0;
		g = 0;
		b = 0;
		a = 1.0;
	}

	/**
	 * RGB / RGBA construct parameters. Alpha is optional, but defaults to 1.0
	 */
	_FORCE_INLINE_ Color(real_t p_r, real_t p_g, real_t p_b, real_t p_a = real_t(1)) {
		r = p_r;
		g = p_g;
		b = p_b;
		a = p_a;
	}
};

bool Color::operator<(const Color &p_color) const {
	if (r == p_color.r) {
		if (g == p_color.g) {
			if (b == p_color.b) {
				return (a < p_color.a);
			} else {
				return (b < p_color.b);
			}
		} else {
			return g < p_color.g;
		}
	} else {
		return r < p_color.r;
	}
}

#endif