template <typename T>
uint32_t Color_t<T>::to_argb32() const {
	uint32_t c = (uint8_t)std::round(a * 255);
	c <<= 8;
	c |= (uint8_t)std::round(r * 255);
	c <<= 8;
	c |= (uint8_t)std::round(g * 255);
	c <<= 8;
	c |= (uint8_t)std::round(b * 255);

	return c;
}

template <typename T>
uint32_t Color_t<T>::to_abgr32() const {
	uint32_t c = (uint8_t)std::round(a * 255);
	c <<= 8;
	c |= (uint8_t)std::round(b * 255);
	c <<= 8;
	c |= (uint8_t)std::round(g * 255);
	c <<= 8;
	c |= (uint8_t)std::round(r * 255);

	return c;
}

template <typename T>
uint32_t Color_t<T>::to_rgba32() const {
	uint32_t c = (uint8_t)std::round(r * 255);
	c <<= 8;
	c |= (uint8_t)std::round(g * 255);
	c <<= 8;
	c |= (uint8_t)std::round(b * 255);
	c <<= 8;
	c |= (uint8_t)std::round(a * 255);	return c;
}

template <typename T>
uint64_t Color_t<T>::to_abgr64() const {
	uint64_t c = (uint16_t)std::round(a * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(b * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(g * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(r * 65535);

	return c;
}

template <typename T>
uint64_t Color_t<T>::to_argb64() const {
	uint64_t c = (uint16_t)std::round(a * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(r * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(g * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(b * 65535);

	return c;
}

template <typename T>
uint64_t Color_t<T>::to_rgba64() const {
	uint64_t c = (uint16_t)std::round(r * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(g * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(b * 65535);
	c <<= 16;
	c |= (uint16_t)std::round(a * 65535);

	return c;
}

template <typename T>
T Color_t<T>::get_h() const {
	T min = MIN(r, g);
	min = MIN(min, b);
	T max = MAX(r, g);
	max = MAX(max, b);

	T delta = max - min;

	if (delta == 0) {
		return T(0);
	}

	T h;
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

template <typename T>
T Color_t<T>::get_s() const {
	T min = MIN(r, g);
	min = MIN(min, b);
	T max = MAX(r, g);
	max = MAX(max, b);

	T delta = max - min;

	return (max != 0) ? (delta / max) : T();
}

template <typename T>
T Color_t<T>::get_v() const {
	T max = MAX(r, g);
	max = MAX(max, b);
	return max;
}

template <typename T>
void Color_t<T>::set_hsv(T p_h, T p_s, T p_v, T p_alpha) {
	int i;
	T f, p, q, t;
	a = p_alpha;

	if (p_s == 0) {
		// acp_hromatic (grey)
		r = g = b = p_v;
		return;
	}

	p_h *= 6;
	p_h = std::fmod(p_h, T(6));
	i = (int)std::floor(p_h);

	f = p_h - i;
	p = p_v * (1 - p_s);
	q = p_v * (1 - p_s * f);
	t = p_v * (1 - p_s * (1 - f));

	switch (i) {
		case 0: // Red is the dominant Color_t<T>
			r = p_v;
			g = t;
			b = p;
			break;
		case 1: // Green is the dominant Color_t<T>
			r = q;
			g = p_v;
			b = p;
			break;
		case 2:
			r = p;
			g = p_v;
			b = t;
			break;
		case 3: // Blue is the dominant Color_t<T>
			r = p;
			g = q;
			b = p_v;
			break;
		case 4:
			r = t;
			g = p;
			b = p_v;
			break;
		default: // (5) Red is the dominant Color_t<T>
			r = p_v;
			g = p;
			b = q;
			break;
	}
}

template <typename T>
bool Color_t<T>::is_equal_approx(const Color_t<T> &p_Color_t) const {
	return std::is_equal_approx(r, p_Color_t.r) && std::is_equal_approx(g, p_Color_t.g) && std::is_equal_approx(b, p_Color_t.b) && std::is_equal_approx(a, p_Color_t.a);
}

template <typename T>
void Color_t<T>::invert() {
	r = 1.0 - r;
	g = 1.0 - g;
	b = 1.0 - b;
}

template <typename T>
void Color_t<T>::contrast() {
	r = std::fmod(r + 0.5, T(1));
	g = std::fmod(g + 0.5, T(1));
	b = std::fmod(b + 0.5, T(1));
}

template <typename T>
Color_t<T> Color_t<T>::hex(uint32_t p_hex) {
	T a = (T)((p_hex & 0xFF) / 255.0);
	p_hex >>= 8;
	T b = (T)((p_hex & 0xFF) / 255.0);
	p_hex >>= 8;
	T g = (T)((p_hex & 0xFF) / 255.0);
	p_hex >>= 8;
	T r = (T)((p_hex & 0xFF) / 255.0);

	return Color_t<T>(r, g, b, a);
}

template <typename T>
Color_t<T> Color_t<T>::hex64(uint64_t p_hex) {
	T a = (T)((p_hex & 0xFFFF) / 65535.0);
	p_hex >>= 16;
	T b = (T)((p_hex & 0xFFFF) / 65535.0);
	p_hex >>= 16;
	T g = (T)((p_hex & 0xFFFF) / 65535.0);
	p_hex >>= 16;
	T r = (T)((p_hex & 0xFFFF) / 65535.0);

	return Color_t<T>(r, g, b, a);
}

template <typename T>
Color_t<T> Color_t<T>::inverted() const {
	Color_t<T> c = *this;
	c.invert();
	return c;
}

template <typename T>
Color_t<T> Color_t<T>::contrasted() const {
	Color_t<T> c = *this;
	c.contrast();
	return c;
}

template <typename T>
Color_t<T> Color_t<T>::from_hsv(T p_h, T p_s, T p_v, T p_a) const {
	Color_t<T> c;
	c.set_hsv(p_h, p_s, p_v, p_a);
	return c;
}

template <typename T>
Color_t<T> Color_t<T>::operator+(const Color_t<T> &p_Color_t) const {
	return Color_t<T>(
			r + p_Color_t.r,
			g + p_Color_t.g,
			b + p_Color_t.b,
			a + p_Color_t.a);
}

template <typename T>
void Color_t<T>::operator+=(const Color_t<T> &p_Color_t) {
	r = r + p_Color_t.r;
	g = g + p_Color_t.g;
	b = b + p_Color_t.b;
	a = a + p_Color_t.a;
}

template <typename T>
Color_t<T> Color_t<T>::operator-(const Color_t<T> &p_Color_t) const {
	return Color_t<T>(
			r - p_Color_t.r,
			g - p_Color_t.g,
			b - p_Color_t.b,
			a - p_Color_t.a);
}

template <typename T>
void Color_t<T>::operator-=(const Color_t<T> &p_Color_t) {
	r = r - p_Color_t.r;
	g = g - p_Color_t.g;
	b = b - p_Color_t.b;
	a = a - p_Color_t.a;
}

template <typename T>
Color_t<T> Color_t<T>::operator*(const Color_t<T> &p_Color_t) const {
	return Color_t<T>(
			r * p_Color_t.r,
			g * p_Color_t.g,
			b * p_Color_t.b,
			a * p_Color_t.a);
}

template <typename T>
Color_t<T> Color_t<T>::operator*(const T &rvalue) const {
	return Color_t<T>(
			r * rvalue,
			g * rvalue,
			b * rvalue,
			a * rvalue);
}

template <typename T>
void Color_t<T>::operator*=(const Color_t<T> &p_Color_t) {
	r = r * p_Color_t.r;
	g = g * p_Color_t.g;
	b = b * p_Color_t.b;
	a = a * p_Color_t.a;
}

template <typename T>
void Color_t<T>::operator*=(const T &rvalue) {
	r = r * rvalue;
	g = g * rvalue;
	b = b * rvalue;
	a = a * rvalue;
}

template <typename T>
Color_t<T> Color_t<T>::operator/(const Color_t<T> &p_Color_t) const {
	return Color_t<T>(
			r / p_Color_t.r,
			g / p_Color_t.g,
			b / p_Color_t.b,
			a / p_Color_t.a);
}

template <typename T>
Color_t<T> Color_t<T>::operator/(const T &rvalue) const {
	return Color_t<T>(
			r / rvalue,
			g / rvalue,
			b / rvalue,
			a / rvalue);
}

template <typename T>
void Color_t<T>::operator/=(const Color_t<T> &p_Color_t) {
	r = r / p_Color_t.r;
	g = g / p_Color_t.g;
	b = b / p_Color_t.b;
	a = a / p_Color_t.a;
}

template <typename T>
void Color_t<T>::operator/=(const T &rvalue) {
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

template <typename T>
Color_t<T> Color_t<T>::operator-() const {
	return Color_t<T>(
			1.0 - r,
			1.0 - g,
			1.0 - b,
			1.0 - a);
}
