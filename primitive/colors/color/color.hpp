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
#include "primitive/colors/rgb/rgb.hpp"

// https://zhuanlan.zhihu.com/p/659818064 色彩空间学习笔记——关于RGB/sRGB/XYZ/Lab/Luv那点事儿（一）


namespace ymd{

namespace details{
template<typename Obj, typename T>
constexpr void set_hsv(Obj & obj, const T p_h,const T p_s, const T p_v){

    if (p_s == 0) {
        obj.r = p_v;
        obj.g = p_v;
        obj.b = p_v;
        return;
    }

    const T p_h_temp = frac(p_h) * 6;
    const size_t i = static_cast<size_t>(p_h_temp);

    const T f = p_h_temp - i;
    const T p = p_v * (1 - p_s);
    const T q = p_v * (1 - p_s * f);
    const T t = p_v * (1 - p_s * (1 - f));

    switch (i) {
        case 0: // Red is the dominant
            obj.r = p_v;
            obj.g = t;
            obj.b = p;
            break;
        case 1: // Green is the dominant
            obj.r = q;
            obj.g = p_v;
            obj.b = p;
            break;
        case 2:
            obj.r = p;
            obj.g = p_v;
            obj.b = t;
            break;
        case 3: // Blue is the dominant
            obj.r = p;
            obj.g = q;
            obj.b = p_v;
            break;
        case 4:
            obj.r = t;
            obj.g = p;
            obj.b = p_v;
            break;
        default: // (5) Red is the dominant
            obj.r = p_v;
            obj.g = p;
            obj.b = q;
            break;
    }
}

template<typename T, typename Dst, typename Src>
constexpr void rgb2srgb(Dst& srgb, const Src& rgb) {
    constexpr T threshold = static_cast<T>(0.0031308f);
    constexpr T linear_scale = static_cast<T>(12.92f);
    constexpr T gamma = static_cast<T>(1.0f / 2.4f);
    constexpr T scale = static_cast<T>(1.055f);
    constexpr T offset = static_cast<T>(0.055f);
    
    auto conv = [&](T x) -> T {  // 内部使用 T 计算
        if(x < threshold) 
            return linear_scale * x;
        else
            return scale * pow(x, gamma) - offset;
    };

    srgb.r = (conv(static_cast<T>(rgb.r)));
    srgb.g = (conv(static_cast<T>(rgb.g)));
    srgb.b = (conv(static_cast<T>(rgb.b)));
}

template<typename T, typename Dst, typename Src>
constexpr void srgb2rgb(Dst& rgb, const Src& srgb) {
    constexpr T threshold = static_cast<T>(0.04045f);
    constexpr T linear_scale = static_cast<T>(1.0f / 12.92f);
    constexpr T gamma = static_cast<T>(2.4f);
    constexpr T scale = static_cast<T>(1.0f / 1.055f);
    constexpr T offset = static_cast<T>(0.055f);
    
    auto conv = [&](T x) -> T {
        return x < threshold ?
            x * linear_scale :
            pow((x + offset) * scale, gamma);
    };

    rgb.r = (conv(static_cast<T>(srgb.r)));
    rgb.g = (conv(static_cast<T>(srgb.g)));
    rgb.b = (conv(static_cast<T>(srgb.b)));
}
}

template <typename T>
requires std::is_arithmetic_v<T>
struct RGBA {
public:
    T r;
    T g;
    T b;
    T a;


    [[nodiscard]] __fast_inline static constexpr RGBA from_pure_black(){
        return RGBA<T>(T(0), T(0), T(0), T(1));
    }

    [[nodiscard]] __fast_inline static constexpr RGBA from_uninitialized(){
        return RGBA<T>();
    }

	constexpr __fast_inline RGBA(T p_r, T p_g, T p_b, T p_a = T(1)) {
		r = static_cast<T>(p_r);
		g = static_cast<T>(p_g);
		b = static_cast<T>(p_b);
		a = static_cast<T>(p_a);
	}

	constexpr RGBA(const RGBA &) = default;
	constexpr RGBA(RGBA &&) = default;

	constexpr RGBA &operator=(const RGBA &) = default;
	constexpr RGBA &operator=(RGBA &&) = default;
    
	[[nodiscard]] constexpr bool operator==(const RGBA & other) const {
        return (r == other.r && g == other.g && b == other.b && a == other.a);
    }

	[[nodiscard]] constexpr bool operator!=(const RGBA & other) const {
        return (r != other.r || g != other.g || b != other.b || a != other.a);
    }

    [[nodiscard]] constexpr 
    T get_h() const {
        T min = MIN(r, g, b);
        T max = MAX(r, g, b);

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

        h /= 6;
        if (h < 0) {
            h += 1.0;
        }

        return h;
    }

    [[nodiscard]] constexpr 
    T get_s() const {
        T min = MIN(r, g);
        min = MIN(min, b);
        T max = MAX(r, g);
        max = MAX(max, b);

        T delta = max - min;

        return (max != 0) ? (delta / max) : T();
    }

    [[nodiscard]] constexpr 
    T get_v() const {
        T max = MAX(r, g);
        max = MAX(max, b);
        return max;
    }


	__fast_inline constexpr T &operator[](const uint8_t idx) {
        static T default_value = T();
		return (idx < 4) ? *(&r + idx) : default_value;
	}

	__fast_inline constexpr T &operator[](const uint8_t idx) const {
        static const T default_value = T();
		return (idx < 4) ? *(&r + idx) : default_value;
	}

	constexpr RGBA operator-() const;

    __fast_inline constexpr RGBA &operator+=(const RGBA<auto> &other) {
        r += static_cast<T>(other.r);
        g += static_cast<T>(other.g);
        b += static_cast<T>(other.b);
        a += static_cast<T>(other.a);
        return *this;
    }

    __fast_inline constexpr RGBA &operator-=(const RGBA<auto> &other) {
        r -= static_cast<T>(other.r);
        g -= static_cast<T>(other.g);
        b -= static_cast<T>(other.b);
        a -= static_cast<T>(other.a);
        return *this;
    }

    __fast_inline constexpr RGBA &operator*=(const RGBA<auto> &other) {
        r *= static_cast<T>(other.r);
        g *= static_cast<T>(other.g);
        b *= static_cast<T>(other.b);
        a *= static_cast<T>(other.a);
        return *this;
    }

    __fast_inline constexpr RGBA & operator*=(const auto &rvalue) {
        r = r * rvalue;
        g = g * rvalue;
        b = b * rvalue;
        a = a * rvalue;
        return *this;
    }

    __fast_inline constexpr RGBA & operator/=(const auto &rvalue) {
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

	constexpr bool is_equal_approx(const RGBA &other) const;
    constexpr void invert() {
        r = static_cast<T>(1) - r;
        g = static_cast<T>(1) - g;
        b = static_cast<T>(1) - b;
    }

    constexpr void contrast() {
        r = fmod(r + static_cast<T>(0.5), T(1));
        g = fmod(g + static_cast<T>(0.5), T(1));
        b = fmod(b + static_cast<T>(0.5), T(1));
    }

    [[nodiscard]] constexpr 
    RGBA<T> inverted() const {
        RGBA<T> c = *this;
        c.invert();
        return c;
    }

    [[nodiscard]] constexpr 
    RGBA<T> contrasted() const {
        RGBA<T> c = *this;
        c.contrast();
        return c;
    }

	[[nodiscard]] __fast_inline constexpr RGBA lerp(const RGBA &p_to, T p_weight) const {
		RGBA ret = *this;

		ret.r += (p_weight * (p_to.r - r));
		ret.g += (p_weight * (p_to.g - g));
		ret.b += (p_weight * (p_to.b - b));
		ret.a += (p_weight * (p_to.a - a));

		return ret;
	}

	[[nodiscard]] __fast_inline constexpr RGBA darkened(T p_amount) const {
		RGBA ret = *this;
		ret.r = ret.r * (static_cast<T>(1) - p_amount);
		ret.g = ret.g * (static_cast<T>(1) - p_amount);
		ret.b = ret.b * (static_cast<T>(1) - p_amount);
		return ret;
	}

	[[nodiscard]] __fast_inline constexpr RGBA lightened(T p_amount) const {
		RGBA ret = *this;
		ret.r = ret.r + (static_cast<T>(1) - ret.r) * p_amount;
		ret.g = ret.g + (static_cast<T>(1) - ret.g) * p_amount;
		ret.b = ret.b + (static_cast<T>(1) - ret.b) * p_amount;
		return ret;
	}

	[[nodiscard]] __fast_inline constexpr RGBA blend(const RGBA &p_over) const {
		RGBA ret;
		T sa = static_cast<T>(1.0) - p_over.a;
		ret.a = a * sa + p_over.a;
		if (ret.a == 0) {
			return RGBA(T(0), T(0), T(0), T(0));
		} else {
            const T inv_a = static_cast<T>(1.0) / ret.a;
			ret.r = (r * a * sa + p_over.r * p_over.a) * inv_a;
			ret.g = (g * a * sa + p_over.g * p_over.a) * inv_a;
			ret.b = (b * a * sa + p_over.b * p_over.a) * inv_a;
		}
		return ret;
	}

	[[nodiscard]] static constexpr RGBA from_hsv(
        const T p_h, 
        const T p_s = 1, 
        const T p_v = 1, 
        const T p_a = 1
    ){
        RGBA<T> c = RGBA<T>::from_uninitialized();
        details::set_hsv(c, p_h, p_s, p_v);
        c.a = p_a;
        return c;
    }


    [[nodiscard]] __fast_inline constexpr operator bool() const{
        return !((a == T(0)) or (r == T(0) and g == T(0) and b == T(0)));
    }

    [[nodiscard]] __fast_inline constexpr RGB565 to_rgb565() const {
        return RGB565::from_r5g6b5(
            static_cast<uint8_t>(r * a * 31),
            static_cast<uint8_t>(g * a * 63),
            static_cast<uint8_t>(b * a * 31)
        );
    }

    [[nodiscard]] __fast_inline constexpr RGB888 to_rgb888() const {
        return RGB888::from_r8g8b8(
            static_cast<uint8_t>(r * a * 255),
            static_cast<uint8_t>(g * a * 255),
            static_cast<uint8_t>(b * a * 255)
        );
    }

    friend OutputStream & operator<<(OutputStream & os, const RGBA<auto> & rgba){
        const auto splt = os.splitter();
        return os << os.brackets<'('>() << rgba.r << splt
            << rgba.g << splt 
            << rgba.b << splt 
            << rgba.a 
            << os.brackets<')'>();
    }

private:
	constexpr RGBA() = default;

};

template<typename T>
struct RGB{
    T r, g, b;

    [[nodiscard]] constexpr static RGB from_hsv(const T p_h, const T p_s, const T p_v){
        RGB ret = RGB::from_uninitialized();
        details::set_hsv(ret, p_h, p_s, p_v);
        return ret;
    }

    [[nodiscard]] constexpr static RGB from_uninitialized(){
        return RGB();
    }

    constexpr RGB(const T p_r, const T p_g, const T p_b) : r(p_r), g(p_g), b(p_b) {}

    constexpr RGB & operator *= (const T val){
        r *= val;
        g *= val;
        b *= val;

        return *this;
    }

    constexpr RGB & operator *= (const RGB & val){
        r *= val.r;
        g *= val.g;
        b *= val.b;

        return *this;
    }

    constexpr RGB & operator /= (const T val){
        const auto inv_val = 1 / val;
        r *= inv_val;
        g *= inv_val;
        b *= inv_val;

        return *this;
    }

    [[nodiscard]] constexpr RGB operator * (const T val) const {
        auto ret = *this;
        ret *= val;
        return ret;
    }

    [[nodiscard]] constexpr RGB operator * (const RGB & val) const {
        auto ret = *this;
        ret *= val;
        return ret;
    }


    [[nodiscard]] constexpr RGB operator / (const T val) const {
        auto ret = *this;
        ret /= val;
        return ret;
    }

    constexpr RGB & operator += (const RGB & val){
        r += val.r;
        g += val.g;
        b += val.b;

        return *this;
    }

    [[nodiscard]] constexpr RGB operator + (const RGB & val) const {
        auto ret = *this;
        ret += val;
        return ret;
    }

    constexpr RGB & operator -= (const RGB & val){
        r -= val.r;
        g -= val.g;
        b -= val.b;

        return *this;
    }

    [[nodiscard]] constexpr RGB operator - (const RGB & val) const {
        auto ret = *this;
        ret -= val;
        return ret;
    }

    [[nodiscard]] constexpr RGB operator - () const {
        return RGB(-r,-g,-b);
    }


    friend OutputStream & operator<<(OutputStream & os, const RGB<auto> & rgb){
        const auto splt = os.splitter();
        return os << os.brackets<'('>() << rgb.r << splt
            << rgb.g << splt 
            << rgb.b 
            << os.brackets<')'>();
    }
private:
    RGB() = default;
};


template<typename T>
struct sRGB{
    T r, g, b;

    static constexpr sRGB<T> from_uninitialized() {
        return sRGB<T>();
    }

    constexpr sRGB(const auto _r, const auto _g, const auto _b):
        r(static_cast<T>(_r)),
        g(static_cast<T>(_g)),
        b(static_cast<T>(_b))
    {}

    static constexpr sRGB<T> from_linear(const RGB<T> & rgb){
        auto ret = sRGB<T>::from_uninitialized();
        details::rgb2srgb<T>(ret, rgb);
        return ret;
    }

    constexpr RGB<T> to_linear() const{
        auto ret = RGB<T>::from_uninitialized();
        details::srgb2rgb<T>(ret, *this);
        return ret;
    }


    // 在sRGB空间安全的操作
    [[nodiscard]] constexpr sRGB<T> inverted() const {
        return sRGB(T(1) - r, T(1) - g, T(1) - b);
    }

    [[nodiscard]] constexpr T luminance() const {
        // sRGB亮度公式 (使用sRGB分量)
        return T(0.2126) * r + T(0.7152) * g + T(0.0722) * b;
    }

    template<size_t I>
    [[nodiscard]] constexpr T& get() {
        return get_element<I>(*this);
    }


    template<size_t I>
    [[nodiscard]] constexpr const T& get() const {
        return get_element<I>(*this);
    }

    friend OutputStream & operator<<(OutputStream & os, const sRGB<auto> & srgb){
        const auto splt = os.splitter();
        return os << os.brackets<'('>() << srgb.r << splt
            << srgb.g << splt 
            << srgb.b 
            << os.brackets<')'>();
    }
private:
    sRGB() = default;

    template<size_t I>
    requires (I < 3)
    [[nodiscard]] static constexpr auto & get_element(auto & self) {
        if constexpr (I == 0) return self.r;
        else if constexpr (I == 1) return self.g;
        else return self.b;
    }

};

}

#include "color.tpp"