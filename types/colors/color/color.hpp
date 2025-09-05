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

#include "core/math/real.hpp"
#include "core/stream/ostream.hpp"
#include "types/colors/rgb/rgb.hpp"


namespace ymd{

template <typename T>
requires std::is_arithmetic_v<T>
struct Color {
public:
    T r = T(0);
    T g = T(0);
    T b = T(0);
    T a = T(1);

	Color() = default;

	
	constexpr __fast_inline Color(T p_r, T p_g, T p_b, T p_a = T(1)) {
		r = static_cast<T>(p_r);
		g = static_cast<T>(p_g);
		b = static_cast<T>(p_b);
		a = static_cast<T>(p_a);
	}

	Color(const Color &) = default;
	Color(Color &&) = default;

	Color &operator=(const Color &) = default;
	Color &operator=(Color &&) = default;
    
	constexpr bool operator==(const Color &p_Color) const { return (r == p_Color.r && g == p_Color.g && b == p_Color.b && a == p_Color.a); }
	constexpr bool operator!=(const Color &p_Color) const { return (r != p_Color.r || g != p_Color.g || b != p_Color.b || a != p_Color.a); }

	constexpr uint32_t to_rgba32() const;
	constexpr uint32_t to_argb32() const;
	constexpr uint32_t to_abgr32() const;
	constexpr uint64_t to_rgba64() const;
	constexpr uint64_t to_argb64() const;
	constexpr uint64_t to_abgr64() const;
	constexpr T get_h() const;
	constexpr T get_s() const;
	constexpr T get_v() const;
    
    // template<typename U>
    constexpr void set_hsv(const T _p_h, const T _p_s, const T _p_v, const T _p_alpha = T(1));

	__fast_inline constexpr T &operator[](const uint8_t idx) {
        static T default_value = T();
		return (idx < 4) ? *(&r + idx) : default_value;
	}
	__fast_inline constexpr T &operator[](const uint8_t idx) const {
        static const T default_value = T();
		return (idx < 4) ? *(&r + idx) : default_value;
	}

	constexpr Color operator-() const;

    // template <typename U>
    __fast_inline constexpr Color &operator+=(const Color<auto> &p_Color) {
        r += static_cast<T>(p_Color.r);
        g += static_cast<T>(p_Color.g);
        b += static_cast<T>(p_Color.b);
        a += static_cast<T>(p_Color.a);
        return *this;
    }

    // template <typename U>
    __fast_inline constexpr Color &operator-=(const Color<auto> &p_Color) {
        r -= static_cast<T>(p_Color.r);
        g -= static_cast<T>(p_Color.g);
        b -= static_cast<T>(p_Color.b);
        a -= static_cast<T>(p_Color.a);
        return *this;
    }

    // template <typename U>
    __fast_inline constexpr Color &operator*=(const Color<auto> &p_Color) {
        r *= static_cast<T>(p_Color.r);
        g *= static_cast<T>(p_Color.g);
        b *= static_cast<T>(p_Color.b);
        a *= static_cast<T>(p_Color.a);
        return *this;
    }

    // template <typename U>
    __fast_inline constexpr Color & operator*=(const auto &rvalue) {
        r = r * rvalue;
        g = g * rvalue;
        b = b * rvalue;
        a = a * rvalue;
        return *this;
    }

    // template<typename U>
    __fast_inline constexpr Color & operator/=(const Color<auto> &p_Color) {
        r /= static_cast<T>(p_Color.r);
        g /= static_cast<T>(p_Color.g);
        b /= static_cast<T>(p_Color.b);
        a /= static_cast<T>(p_Color.a);
        return *this;
    }

    __fast_inline constexpr Color & operator/=(const auto &rvalue) {
        if (rvalue == 0) {
            r = 1.0;
            g = 1.0;
            b = 1.0;
            a = 1.0;
        } else {
            r /= rvalue;
            g /= rvalue;
            b /= rvalue;
            a /= rvalue;
        }
        return *this;
    };

	constexpr bool is_equal_approx(const Color &p_Color) const;
	constexpr void invert();
	constexpr void contrast();
	constexpr Color inverted() const;
	constexpr Color contrasted() const;

	__fast_inline constexpr Color linear_interpolate(const Color &p_to, T p_weight) const {
		Color res = *this;

		res.r += (p_weight * (p_to.r - r));
		res.g += (p_weight * (p_to.g - g));
		res.b += (p_weight * (p_to.b - b));
		res.a += (p_weight * (p_to.a - a));

		return res;
	}

	__fast_inline constexpr Color darkened(T p_amount) const {
		Color res = *this;
		res.r = res.r * (1.0f - p_amount);
		res.g = res.g * (1.0f - p_amount);
		res.b = res.b * (1.0f - p_amount);
		return res;
	}

	__fast_inline constexpr Color lightened(T p_amount) const {
		Color res = *this;
		res.r = res.r + (1.0f - res.r) * p_amount;
		res.g = res.g + (1.0f - res.g) * p_amount;
		res.b = res.b + (1.0f - res.b) * p_amount;
		return res;
	}

	__fast_inline constexpr Color blend(const Color &p_over) const {
		Color res;
		T sa = 1.0 - p_over.a;
		res.a = a * sa + p_over.a;
		if (res.a == 0) {
			return Color(T(), T(), T(), T());
		} else {
			res.r = (r * a * sa + p_over.r * p_over.a) / res.a;
			res.g = (g * a * sa + p_over.g * p_over.a) / res.a;
			res.b = (b * a * sa + p_over.b * p_over.a) / res.a;
		}
		return res;
	}

	__fast_inline constexpr Color to_linear() const {
		return Color(
				r < 0.04045 ? r * (1.0 / 12.92) : pow((r + 0.055) * (1.0 / (1 + 0.055)), T(2.4)),
				g < 0.04045 ? g * (1.0 / 12.92) : pow((g + 0.055) * (1.0 / (1 + 0.055)), T(2.4)),
				b < 0.04045 ? b * (1.0 / 12.92) : pow((b + 0.055) * (1.0 / (1 + 0.055)), T(2.4)),
				a);
	}

	__fast_inline constexpr Color to_srgb() const {
		return Color(
				r < 0.0031308 ? 12.92 * r : (1.0 + 0.055) * pow(r, T(1.0f / 2.4f)) - T(0.055),
				g < 0.0031308 ? 12.92 * g : (1.0 + 0.055) * pow(g, T(1.0f / 2.4f)) - T(0.055),
				b < 0.0031308 ? 12.92 * b : (1.0 + 0.055) * pow(b, T(1.0f / 2.4f)) - T(0.055), a);
	}

	constexpr Color hex(uint32_t p_hex);
	constexpr Color hex64(uint64_t p_hex);

	constexpr static Color<T> from_hsv(const T p_h, const T p_s = 1, const T p_v = 1, const T p_a = 1);

    __fast_inline constexpr operator bool() const{
        return !((a == T(0)) or (r == T(0) and g == T(0) and b == T(0)));
    }
    __fast_inline constexpr operator RGB565() const {
        return RGB565::from_r5g6b5(
            uint8_t(r * a * 31),
            uint8_t(g * a * 63),
            uint8_t(b * a * 31)
        );
    }

    __fast_inline constexpr operator RGB888() const {
        return RGB888(
            (uint8_t)(r * a * 255),
            (uint8_t)(g * a * 255),
            (uint8_t)(b * a * 255));
    }
};

__fast_inline OutputStream & operator<<(OutputStream & os, const Color<auto> & value){
	const auto splt = os.splitter();
    return os << os.brackets<'('>() << value.r << splt << value.g << splt << value.b << splt << value.a << os.brackets<')'>();
}


template<arithmetic T>
Color() -> Color<T>;

template<typename T>
struct RGB{
    T r, g, b;

    RGB & operator *= (const T val){
        r *= val;
        g *= val;
        b *= val;

        return *this;
    }

    RGB & operator *= (const RGB & val){
        r *= val.r;
        g *= val.g;
        b *= val.b;

        return *this;
    }

    RGB & operator /= (const T val){
        const auto inv_val = 1 / val;
        r *= inv_val;
        g *= inv_val;
        b *= inv_val;

        return *this;
    }

    RGB operator * (const T val) const {
        auto ret = RGB(*this);
        ret *= val;
        return ret;
    }

    RGB operator * (const RGB & val) const {
        auto ret = RGB(*this);
        ret *= val;
        return ret;
    }


    RGB operator / (const T val) const {
        auto ret = RGB(*this);
        ret /= val;
        return ret;
    }

    RGB & operator += (const RGB & val){
        r += val.r;
        g += val.g;
        b += val.b;

        return *this;
    }

    RGB operator + (const RGB & val) const {
        auto ret = RGB(*this);
        ret += val;
        return ret;
    }

    RGB & operator -= (const RGB & val){
        r -= val.r;
        g -= val.g;
        b -= val.b;

        return *this;
    }

    RGB operator - (const RGB & val) const {
        auto ret = RGB(*this);
        ret -= val;
        return ret;
    }

    RGB operator - () const {
        return RGB(-r,-g,-b);
    }
};


}

#include "color.tpp"