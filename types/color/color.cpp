
#include "color.hpp"

uint32_t Color::to_argb32() const {
	uint32_t c = (uint8_t)std::round(a * 255);
	c <<= 8;
	c |= (uint8_t)std::round(r * 255);
	c <<= 8;
	c |= (uint8_t)std::round(g * 255);
	c <<= 8;
	c |= (uint8_t)std::round(b * 255);

	return c;
}

uint32_t Color::to_abgr32() const {
	uint32_t c = (uint8_t)std::round(a * 255);
	c <<= 8;
	c |= (uint8_t)std::round(b * 255);
	c <<= 8;
	c |= (uint8_t)std::round(g * 255);
	c <<= 8;
	c |= (uint8_t)std::round(r * 255);

	return c;
}

uint32_t Color::to_rgba32() const {
	uint32_t c = (uint8_t)std::round(r * 255);
	c <<= 8;
	c |= (uint8_t)std::round(g * 255);
	c <<= 8;
	c |= (uint8_t)std::round(b * 255);
	c <<= 8;
	c |= (uint8_t)std::round(a * 255);

	return c;
}

uint64_t Color::to_abgr64() const {
	uint64_t c = (uint16_t)std::round(a * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(b * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(g * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(r * 65535);

	return c;
}

uint64_t Color::to_argb64() const {
	uint64_t c = (uint16_t)std::round(a * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(r * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(g * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(b * 65535);

	return c;
}

uint64_t Color::to_rgba64() const {
	uint64_t c = (uint16_t)std::round(r * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(g * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(b * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(a * 65535);

	return c;
}

real_t Color::get_h() const {
	real_t min = MIN(r, g);
	min = MIN(min, b);
	real_t max = MAX(r, g);
	max = MAX(max, b);

	real_t delta = max - min;

	if (delta == 0) {
		return real_t(0);
	}

	real_t h;
	if (r == max) {
		h = (g - b) / delta; // between yellow & magenta
	} else if (g == max) {
		h = 2 + (b - r) / delta; // between cyan & yellow
	} else {
		h = 4 + (r - g) / delta; // between magenta & cyan
	}

	h /= 6.0;
	if (h < 0) {
		h += 1.0;
	}

	return h;
}

real_t Color::get_s() const {
	real_t min = MIN(r, g);
	min = MIN(min, b);
	real_t max = MAX(r, g);
	max = MAX(max, b);

	real_t delta = max - min;

	return (max != 0) ? (delta / max) : real_t();
}

real_t Color::get_v() const {
	real_t max = MAX(r, g);
	max = MAX(max, b);
	return max;
}

void Color::set_hsv(real_t p_h, real_t p_s, real_t p_v, real_t p_alpha) {
	int i;
	real_t f, p, q, t;
	a = p_alpha;

	if (p_s == 0) {
		// acp_hromatic (grey)
		r = g = b = p_v;
		return;
	}

	p_h *= 6;
	p_h = std::fmod(p_h, real_t(6));
	i = (int)std::floor(p_h);

	f = p_h - i;
	p = p_v * (1 - p_s);
	q = p_v * (1 - p_s * f);
	t = p_v * (1 - p_s * (1 - f));

	switch (i) {
		case 0: // Red is the dominant color
			r = p_v;
			g = t;
			b = p;
			break;
		case 1: // Green is the dominant color
			r = q;
			g = p_v;
			b = p;
			break;
		case 2:
			r = p;
			g = p_v;
			b = t;
			break;
		case 3: // Blue is the dominant color
			r = p;
			g = q;
			b = p_v;
			break;
		case 4:
			r = t;
			g = p;
			b = p_v;
			break;
		default: // (5) Red is the dominant color
			r = p_v;
			g = p;
			b = q;
			break;
	}
}

bool Color::is_equal_approx(const Color &p_color) const {
	return std::is_equal_approx(r, p_color.r) && std::is_equal_approx(g, p_color.g) && std::is_equal_approx(b, p_color.b) && std::is_equal_approx(a, p_color.a);
}

void Color::invert() {
	r = 1.0 - r;
	g = 1.0 - g;
	b = 1.0 - b;
}
void Color::contrast() {
	r = std::fmod(r + 0.5, real_t(1));
	g = std::fmod(g + 0.5, real_t(1));
	b = std::fmod(b + 0.5, real_t(1));
}

Color Color::hex(uint32_t p_hex) {
	real_t a = (real_t)((p_hex & 0xFF) / 255.0);
	p_hex >>= 8;
	real_t b = (real_t)((p_hex & 0xFF) / 255.0);
	p_hex >>= 8;
	real_t g = (real_t)((p_hex & 0xFF) / 255.0);
	p_hex >>= 8;
	real_t r = (real_t)((p_hex & 0xFF) / 255.0);

	return Color(r, g, b, a);
}

Color Color::hex64(uint64_t p_hex) {
	real_t a = (real_t)((p_hex & 0xFFFF) / 65535.0);
	p_hex >>= 16;
	real_t b = (real_t)((p_hex & 0xFFFF) / 65535.0);
	p_hex >>= 16;
	real_t g = (real_t)((p_hex & 0xFFFF) / 65535.0);
	p_hex >>= 16;
	real_t r = (real_t)((p_hex & 0xFFFF) / 65535.0);

	return Color(r, g, b, a);
}

Color Color::inverted() const {
	Color c = *this;
	c.invert();
	return c;
}

Color Color::contrasted() const {
	Color c = *this;
	c.contrast();
	return c;
}

Color Color::from_hsv(real_t p_h, real_t p_s, real_t p_v, real_t p_a) const {
	Color c;
	c.set_hsv(p_h, p_s, p_v, p_a);
	return c;
}

Color Color::operator+(const Color &p_color) const {
	return Color(
			r + p_color.r,
			g + p_color.g,
			b + p_color.b,
			a + p_color.a);
}

void Color::operator+=(const Color &p_color) {
	r = r + p_color.r;
	g = g + p_color.g;
	b = b + p_color.b;
	a = a + p_color.a;
}

Color Color::operator-(const Color &p_color) const {
	return Color(
			r - p_color.r,
			g - p_color.g,
			b - p_color.b,
			a - p_color.a);
}

void Color::operator-=(const Color &p_color) {
	r = r - p_color.r;
	g = g - p_color.g;
	b = b - p_color.b;
	a = a - p_color.a;
}

Color Color::operator*(const Color &p_color) const {
	return Color(
			r * p_color.r,
			g * p_color.g,
			b * p_color.b,
			a * p_color.a);
}

Color Color::operator*(const real_t &rvalue) const {
	return Color(
			r * rvalue,
			g * rvalue,
			b * rvalue,
			a * rvalue);
}

void Color::operator*=(const Color &p_color) {
	r = r * p_color.r;
	g = g * p_color.g;
	b = b * p_color.b;
	a = a * p_color.a;
}

void Color::operator*=(const real_t &rvalue) {
	r = r * rvalue;
	g = g * rvalue;
	b = b * rvalue;
	a = a * rvalue;
}

Color Color::operator/(const Color &p_color) const {
	return Color(
			r / p_color.r,
			g / p_color.g,
			b / p_color.b,
			a / p_color.a);
}

Color Color::operator/(const real_t &rvalue) const {
	return Color(
			r / rvalue,
			g / rvalue,
			b / rvalue,
			a / rvalue);
}

void Color::operator/=(const Color &p_color) {
	r = r / p_color.r;
	g = g / p_color.g;
	b = b / p_color.b;
	a = a / p_color.a;
}

void Color::operator/=(const real_t &rvalue) {
	if (rvalue == 0) {
		r = 1.0;
		g = 1.0;
		b = 1.0;
		a = 1.0;
	} else {
		r = r / rvalue;
		g = g / rvalue;
		b = b / rvalue;
		a = a / rvalue;
	}
};

Color Color::operator-() const {
	return Color(
			1.0 - r,
			1.0 - g,
			1.0 - b,
			1.0 - a);
}
