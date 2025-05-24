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


namespace ymd{

template <typename T> 
constexpr uint32_t Color_t<T>::to_argb32() const {
	uint32_t c = (uint8_t)round(a * 255);
	c <<= 8;
	c |= (uint8_t)round(r * 255);
	c <<= 8;
	c |= (uint8_t)round(g * 255);
	c <<= 8;
	c |= (uint8_t)round(b * 255);

	return c;
}

template <typename T> constexpr 
uint32_t Color_t<T>::to_abgr32() const {
	uint32_t c = (uint8_t)round(a * 255);
	c <<= 8;
	c |= (uint8_t)round(b * 255);
	c <<= 8;
	c |= (uint8_t)round(g * 255);
	c <<= 8;
	c |= (uint8_t)round(r * 255);

	return c;
}

template <typename T> constexpr 
uint32_t Color_t<T>::to_rgba32() const {
	uint32_t c = (uint8_t)round(r * 255);
	c <<= 8;
	c |= (uint8_t)round(g * 255);
	c <<= 8;
	c |= (uint8_t)round(b * 255);
	c <<= 8;
	c |= (uint8_t)round(a * 255);	return c;
}

template <typename T> constexpr 
uint64_t Color_t<T>::to_abgr64() const {
	uint64_t c = (uint16_t)round(a * 65535);
	c <<= 16;
	c |= (uint16_t)round(b * 65535);
	c <<= 16;
	c |= (uint16_t)round(g * 65535);
	c <<= 16;
	c |= (uint16_t)round(r * 65535);

	return c;
}

template <typename T> constexpr 
uint64_t Color_t<T>::to_argb64() const {
	uint64_t c = (uint16_t)round(a * 65535);
	c <<= 16;
	c |= (uint16_t)round(r * 65535);
	c <<= 16;
	c |= (uint16_t)round(g * 65535);
	c <<= 16;
	c |= (uint16_t)round(b * 65535);

	return c;
}

template <typename T> constexpr 
uint64_t Color_t<T>::to_rgba64() const {
	uint64_t c = (uint16_t)round(r * 65535);
	c <<= 16;
	c |= (uint16_t)round(g * 65535);
	c <<= 16;
	c |= (uint16_t)round(b * 65535);
	c <<= 16;
	c |= (uint16_t)round(a * 65535);

	return c;
}

template <typename T> constexpr 
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

template <typename T> constexpr 
T Color_t<T>::get_s() const {
	T min = MIN(r, g);
	min = MIN(min, b);
	T max = MAX(r, g);
	max = MAX(max, b);

	T delta = max - min;

	return (max != 0) ? (delta / max) : T();
}

template <typename T> constexpr 
T Color_t<T>::get_v() const {
	T max = MAX(r, g);
	max = MAX(max, b);
	return max;
}

template <typename T>
constexpr void Color_t<T>::set_hsv(const T p_h, const T p_s, const T p_v, const T p_alpha) {
	a = p_alpha;

	if (p_s == 0) {
		r = g = b = p_v;
		return;
	}

	const auto p_h_temp = frac(p_h) * 6;
	const int i = int(p_h_temp);

	const T f = p_h_temp - i;
	const T p = p_v * (1 - p_s);
	const T q = p_v * (1 - p_s * f);
	const T _t = p_v * (1 - p_s * (1 - f));

	switch (i) {
		case 0: // Red is the dominant Color_t<T>
			r = p_v;
			g = _t;
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
			b = _t;
			break;
		case 3: // Blue is the dominant Color_t<T>
			r = p;
			g = q;
			b = p_v;
			break;
		case 4:
			r = _t;
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

template <typename T> constexpr 
bool Color_t<T>::is_equal_approx(const Color_t<T> &p_Color) const {
	return is_equal_approx(r, p_Color.r) && is_equal_approx(g, p_Color.g) && is_equal_approx(b, p_Color.b) && is_equal_approx(a, p_Color.a);
}

template <typename T> constexpr 
void Color_t<T>::invert() {
	r = 1.0 - r;
	g = 1.0 - g;
	b = 1.0 - b;
}

template <typename T> constexpr 
void Color_t<T>::contrast() {
	r = fmod(r + 0.5, T(1));
	g = fmod(g + 0.5, T(1));
	b = fmod(b + 0.5, T(1));
}

template <typename T> constexpr 
Color_t<T> Color_t<T>::hex(uint32_t p_hex) {
	T _a = (T)((p_hex & 0xFF) / 255.0);
	p_hex >>= 8;
	T _b = (T)((p_hex & 0xFF) / 255.0);
	p_hex >>= 8;
	T _g = (T)((p_hex & 0xFF) / 255.0);
	p_hex >>= 8;
	T _r = (T)((p_hex & 0xFF) / 255.0);

	return Color_t<T>(_r, _g, _b, _a);
}

template <typename T> constexpr 
Color_t<T> Color_t<T>::hex64(uint64_t p_hex) {
	T _a = (T)((p_hex & 0xFFFF) / 65535.0);
	p_hex >>= 16;
	T _b = (T)((p_hex & 0xFFFF) / 65535.0);
	p_hex >>= 16;
	T _g = (T)((p_hex & 0xFFFF) / 65535.0);
	p_hex >>= 16;
	T _r = (T)((p_hex & 0xFFFF) / 65535.0);

	return Color_t<T>(_r, _g, _b, _a);
}

template <typename T> constexpr 
Color_t<T> Color_t<T>::inverted() const {
	Color_t<T> c = *this;
	c.invert();
	return c;
}

template <typename T> constexpr 
Color_t<T> Color_t<T>::contrasted() const {
	Color_t<T> c = *this;
	c.contrast();
	return c;
}

template<typename T>
constexpr Color_t<T> Color_t<T>::from_hsv(const T p_h, const T p_s, const T p_v, const T p_a){
	Color_t<T> c;
	c.set_hsv(p_h, p_s, p_v, p_a);
	return c;
}


#define COLOR_ADD_SUB_MUL_OPERATOR(op) \
template <typename T> \
__fast_inline constexpr Color_t<T> operator op(const Color_t<T> &p_Color, const auto &rvalue){ \
    Color_t<T> final = p_Color; \
    final op##= rvalue; \
    return final; \
}\
\
template <typename T> \
__fast_inline constexpr Color_t<T> operator op(const auto &lvalue, const Color_t<T> &p_Color){ \
    Color_t<T> final = p_Color; \
    final op##= lvalue; \
    return final; \
}\
\
template <typename T> \
__fast_inline constexpr Color_t<T> operator op(const Color_t<T> &p_Color, const Color_t<auto> &d_Color){ \
    Color_t<T> final = p_Color; \
    final op##= d_Color; \
    return final; \
}

COLOR_ADD_SUB_MUL_OPERATOR(+) 
COLOR_ADD_SUB_MUL_OPERATOR(-) 
COLOR_ADD_SUB_MUL_OPERATOR(*) 

#undef COLOR_ADD_SUB_MUL_OPERATOR

template <typename T>
Color_t<T> operator/(const Color_t<T> &p_Color, const auto &rvalue){
    Color_t<T> final = p_Color;
    final /= rvalue;
    return final;
}

template <typename T>
Color_t<T> operator/(const Color_t<T> &p_Color, const Color_t<auto> &d_Color){
    Color_t<T> final = p_Color;
    final /= d_Color;
    return final;
}


template <typename T> constexpr 
Color_t<T> Color_t<T>::operator-() const {
	return Color_t<T>(
			1.0 - r,
			1.0 - g,
			1.0 - b,
			1.0 - a);
}


}