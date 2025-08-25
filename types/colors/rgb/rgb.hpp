#pragma once

#include "core/platform.hpp"
#include "core/math/uint24_t.hpp"
#include "core/math/real.hpp"
#include <tuple>

namespace ymd{
enum class ColorEnum:uint32_t{
    WHITE   = 0xFFFFFF,    // White color
    YELLOW  = 0xFFFF00,    // Yellow color
    BRRED   = 0xFF4500,    // Brown-red color
    PINK    = 0xFFC0CB,    // Pink color
    RED     = 0x0000FF,    // Red color
    BROWN   = 0xA52A2A,    // Brown color
    GRAY    = 0x808080,    // Gray color
    GBLUE   = 0xADD8E6,    // Light blue color
    GREEN   = 0x00FF00,    // Green color
    BLUE    = 0xFF0000,    // Blue color
    BLACK   = 0x000000     // Black color
};

enum class RgbType:uint8_t{
    _RGB332,
    _RGB565,
    _RGB888,
    _Binary,
    _HSV888,
    _LAB888,
    _XYZ888,
    _Gray,
    _IGray,
};



struct RGB332;
struct RGB565;
struct RGB888;
struct Binary;
struct HSV888;
struct LAB888;
struct XYZ888;
struct Gray;
struct IGray;

struct RGB888 {

    uint8_t r;
    uint8_t g;
    uint8_t b;

    __fast_inline constexpr explicit RGB888(const uint32_t _data): 
        r(_data),
        g(_data >> 8),
        b(_data >> 16)     
    {;}

public:
    __fast_inline constexpr explicit RGB888() = default;

    __fast_inline constexpr RGB888(const ColorEnum color): RGB888(uint32_t(color)){;}

    __fast_inline static constexpr RGB888 from_u32(const uint32_t _data){
        return RGB888(_data);
    }
    
    __fast_inline constexpr RGB888(const RGB888 & other): 
        r(other.r),
        g(other.g),
        b(other.b)
    {;} 

    constexpr explicit RGB888(const HSV888 & other);

    __fast_inline static constexpr RGB888 from_r8g8b8(uint8_t r, uint8_t g, uint8_t b){
        return RGB888(r, g, b);
    }

    __fast_inline constexpr uint24_t as_u24() const {return uint24_t(r | (g << 8) | (b << 16));}
private:

    __fast_inline constexpr explicit RGB888(const uint8_t _r, const uint8_t _g, const uint8_t _b):
        r(_r), g(_g), b(_b){;}
};

struct LAB888 {

    uint8_t l;
    int8_t a;
    int8_t b;

public:
    __fast_inline constexpr explicit LAB888() = default;

    __fast_inline constexpr LAB888(const LAB888 & other): 
        l(other.l),
        a(other.a),
        b(other.b)
    {;}

    explicit LAB888(const RGB888 & rgb); 

    explicit operator RGB888() const;
    
    __fast_inline constexpr uint24_t as_u24() const {return uint24_t(l | (a << 8) | (b << 16));}

    __fast_inline static constexpr LAB888 from_l8a8b8(uint8_t l, uint8_t a, uint8_t b){
        return LAB888(l, a, b);
    } 
private:
    __fast_inline constexpr explicit LAB888(const uint8_t _l, const int8_t _a, const int8_t _b):l(_l), a(_a), b(_b){;}


};




struct RGB332{
    union{
        struct{
            uint8_t b : 2;
            uint8_t g : 3;
            uint8_t r : 3;
        };
        uint8_t data;
    };


    __fast_inline constexpr explicit RGB332() : data(0){;}

    __fast_inline constexpr explicit RGB332(const uint8_t _data): data(_data){;}

    __fast_inline static constexpr RGB332 from_r3g3b2(uint8_t r, uint8_t g, uint8_t b){
        return RGB332(r,g,b);
    } 

    __fast_inline constexpr explicit operator uint8_t() const {return data;}

    __fast_inline constexpr uint8_t as_u8() const {
        return data;
    }
private:
    __fast_inline constexpr explicit RGB332(const uint8_t _r, const uint8_t _g, const uint8_t _b): 
        b(_b), g(_g), r(_r){;}
};



struct RGB565{
    uint16_t b : 5;
    uint16_t g : 6;
    uint16_t r : 5;

    using enum ColorEnum;
    __fast_inline constexpr RGB565() = default;
    __fast_inline constexpr RGB565 & operator =(const RGB565 & other) = default;

    constexpr explicit RGB565(const HSV888 & other);
    __fast_inline constexpr RGB565(const ColorEnum color):RGB565(RGB888(color)){;}

    __fast_inline constexpr explicit RGB565(const RGB888 rgb): b(rgb.b >> 3), g(rgb.g >> 2), r(rgb.r >> 3){;}

    
    __fast_inline constexpr explicit RGB565(const Gray & gs);
    
    __fast_inline constexpr explicit RGB565(const Binary & bn);

    __fast_inline static constexpr 
    RGB565 from_r5g6b5(const uint8_t r, const uint8_t g, const uint8_t b){
        RGB565 ret;
        ret.b = b & 0b11111; 
        ret.g = g & 0b111111; 
        ret.r = r & 0b11111;
        return ret;
    }

    __fast_inline static constexpr 
    RGB565 from_u16(const uint16_t raw){
        return std::bit_cast<RGB565>(raw);
    }
    __fast_inline constexpr explicit operator RGB888() const {
        return RGB888::from_r8g8b8(r << 3, g << 2, b << 3);
    }

    __fast_inline constexpr uint16_t as_u16() const {
        return uni(r,g,b);
    }

    // __fast_inline constexpr RGB565 & operator = (const uint16_t data){
    //     uint8_t _r, _g, _b;
    //     std::tie(_r, _g, _b) = seprate(data);
    //     r = _r;
    //     g = _g;
    //     b = _b;
    //     return *this;
    // }

    // __fast_inline constexpr RGB565 & operator = (const RGB565 & rgb){
    //     *this = std::bit_cast<>
    //     return *this;
    // }
private:
    static __fast_inline constexpr ::std::tuple<uint8_t, uint8_t, uint8_t>
    seprate(const uint16_t data){
        return {(data >> 11) & 0x1f, (data >> 5) & 0x3f, data & 0x1f};}
    static __fast_inline constexpr uint16_t uni(const uint8_t _r, const uint8_t _g, const uint8_t _b){
        return ((_r & 0x1f) << 11) | ((_g & 0x3f) << 5) | (_b & 0x1f);}

    __fast_inline constexpr RGB565(const uint8_t _r, const uint8_t _g, const uint8_t _b): 
        b(_b & 0b11111), g(_g & 0b111111), r(_r & 0b11111){;}
};

static_assert(sizeof(RGB565) == 2);




struct HSV888 {

    uint8_t h;
    uint8_t s;
    uint8_t v;

public:
    __fast_inline constexpr HSV888() = default;

    __fast_inline constexpr HSV888(const HSV888 & other) = default;
    constexpr HSV888(const RGB888 & other);
    constexpr HSV888(const RGB565 & other);

    __fast_inline constexpr HSV888(const int _data): 
        h(_data),
        s(_data >> 8),
        v(_data >> 16)
    {;}

    __fast_inline constexpr explicit HSV888(const int _h, const int _s, const int _v):
        h(uint8_t(_h)), s(uint8_t(_s)), v(uint8_t(_v))
    {;}

    __fast_inline constexpr uint24_t as_u24() const {
        return uint24_t(uint32_t(h) << 16 | uint32_t(s) << 8 | uint32_t(v));
    }

};



struct ARGB32{

    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;


    __fast_inline constexpr ARGB32() : ARGB32(0){;}

    __fast_inline constexpr ARGB32(const uint32_t _data): 
        a(_data),
        r(_data >> 8),
        g(_data >> 16),
        b(_data >> 24)
    
    {;}

    __fast_inline constexpr explicit ARGB32(const uint8_t _a, const uint8_t _r, const uint8_t _g, const uint8_t _b):a(_a), r(_r), g(_g), b(_b){;}

    __fast_inline constexpr explicit operator uint32_t() const {return *reinterpret_cast<const uint32_t *>(this);}

};



struct Binary{

    enum Kind:uint8_t{
        WHITE   = 0xFF,  // White color
        BLACK   = 0x00   // Black color
    };

    uint8_t data;

    __fast_inline constexpr explicit Binary(){;}
    __fast_inline constexpr explicit Binary(const Kind kind) : data(kind){;}
    __fast_inline constexpr explicit Binary(const bool _data): data(_data ? 0xff : 0x00){;}
    
    __fast_inline constexpr Binary(const Binary & other) = default;
    __fast_inline constexpr Binary(Binary && other) = default;

    __fast_inline constexpr Binary & operator = (const Kind kind) { data = kind; return *this; }
    __fast_inline constexpr Binary & operator = (const Binary & other) = default;
    __fast_inline constexpr Binary & operator = (Binary && other) = default;
    __fast_inline constexpr explicit Binary(const RGB888 & rgb): 
        data((rgb.r + rgb.g + rgb.b) > 128 * 3 ? 255 : 0){;}

    __fast_inline constexpr bool operator == (const Binary & other) const {return data == other.data;}
    __fast_inline constexpr bool operator == (const Kind kind) const {return data == kind;}

    __fast_inline constexpr bool is_white() const {return data == WHITE;}

    __fast_inline constexpr bool is_black() const {return data == BLACK;}

    __fast_inline constexpr Binary flip() const {
        const uint8_t ret = ~data;
        return Binary(uint8_t(ret));
    }
    __fast_inline constexpr uint8_t to_raw() const {return data;}
    __fast_inline constexpr explicit operator RGB888() const {
        return RGB888::from_r8g8b8(data, data, data);
    }
};



struct Gray{
    uint8_t data;

    enum{
        WHITE   = 0xFF,  // White color
        BLACK   = 0x00   // Black color
    };
    __fast_inline constexpr explicit Gray() : data(0){;}
    __fast_inline constexpr explicit Gray(const uint8_t _data): data(_data){;}

    __fast_inline constexpr explicit Gray(const RGB565 & rgb);
    __fast_inline constexpr explicit operator uint8_t() const {return data;}
    __fast_inline constexpr explicit operator RGB888() const {
        return RGB888::from_r8g8b8(data, data, data);}
    __fast_inline constexpr explicit operator bool() const {return data;}

    __fast_inline constexpr auto operator <=> (const Gray & other) const {
        return data <=> other.data;}
    __fast_inline constexpr bool operator == (const Gray & other) const {
        return data == other.data;}


    __fast_inline constexpr bool is_white() const {return data == uint8_t(0xff);}

    __fast_inline constexpr bool is_black() const {return data == uint8_t(0x00);}

    __fast_inline constexpr Gray flip() const {
        const uint8_t ret = ~data;
        return Gray(uint8_t(ret));
    }

    __fast_inline constexpr Binary to_bina(const Gray & threshold = Gray(128)) const 
        {return Binary(data > (uint8_t)threshold);}
};


struct IGray{
    int8_t data;

    enum{
        WHITE   = 127,  // White color
        BLACK   = 0x00   // Black color
    };
    __fast_inline constexpr IGray() = default;

    __fast_inline constexpr IGray(const int8_t _data): data(_data){;}

    __fast_inline constexpr IGray(const bool bina): data(bina ? 127 : 0x00){;}

    __fast_inline constexpr IGray & operator = (const uint8_t _data){data = _data; return *this;}

    __fast_inline constexpr explicit operator int8_t() const {return data;}

    __fast_inline constexpr explicit operator bool() const {return data;}

    __fast_inline constexpr bool operator > (const IGray & other){return data > other.data;}

    __fast_inline constexpr bool operator < (const IGray & other){return data < other.data;}

    __fast_inline constexpr bool operator >= (const IGray & other){return data >= other.data;}

    __fast_inline constexpr bool operator <= (const IGray & other){return data <= other.data;}

    __fast_inline constexpr Binary to_bina(const uint8_t threshold){return Binary(ABS(data) > threshold);}

    __fast_inline constexpr Binary to_bina_signed(const int8_t threshold){return Binary(data > threshold);}
};


using RGB24 = RGB888;


__fast_inline constexpr Gray::Gray(const RGB565 & rgb):
    data(((rgb.r*77 + rgb.g*150 + rgb.b*29+128) >> 8)){;}

__fast_inline constexpr RGB565::RGB565(const Gray & gs): 
    b(uint8_t(gs) >> 3), g(uint8_t(gs) >> 2), r(uint8_t(gs) >> 3){;}

__fast_inline constexpr RGB565::RGB565(const Binary & bn): 
    RGB565(RGB565::from_u16(bn.is_white() ? 0xffff : 0)){;}

constexpr HSV888::HSV888(const RGB888 & rgb){

    enum{
        HUE_RED = 0,
        HUE_ORANGE = 32,
        HUE_YELLOW = 64,
        HUE_GREEN = 96,
        HUE_AQUA = 128,
        HUE_BLUE = 160,
        HUE_PURPLE = 192,
        HUE_PINK = 224
    };

    auto scale8 = [](const uint8_t i, const uint8_t scale) -> uint8_t {return (((uint16_t)i) * (1+(uint16_t)(scale))) >> 8;};
    auto qsub8 = [](const uint8_t i, const uint8_t j) -> uint8_t {return MAX(int(i) - int(j), 0);};
    auto qadd8 = [](const uint8_t i, const uint8_t j) -> uint8_t {return MIN(int(i) + int(j), 255);};

    #define FIXFRAC8(N,D) (((N)*256)/(D))

    auto sqrt16 = [](uint16_t x) -> uint8_t
    {
        if( x <= 1) {
            return x;
        }

        uint8_t low = 1; // lower bound
        uint8_t hi, mid;

        if( x > 7904) {
            hi = 255;
        } else {
            hi = (x >> 5) + 8; // initial estimate for upper bound
        }

        do {
            mid = (low + hi) >> 1;
            if ((uint16_t)(mid * mid) > x) {
                hi = mid - 1;
            } else {
                if( mid == 255) {
                    return 255;
                }
                low = mid + 1;
            }
        } while (hi >= low);

        return low - 1;
    };

    
    uint8_t r = rgb.r;
    uint8_t g = rgb.g;
    uint8_t b = rgb.b;
    
    // find desaturation
    uint8_t desat = 255;
    if( r < desat) desat = r;
    if( g < desat) desat = g;
    if( b < desat) desat = b;
    
    // remove saturation from all channels
    r -= desat;
    g -= desat;
    b -= desat;
    

    s = 255 - desat;
    
    if( s != 255 ) {
        // undo 'dimming' of saturation
        s = 255 - sqrt16(uint16_t(255-s) << 8);
    }

    if( (r + g + b) == 0) {
        // we pick hue zero for no special reason
        h = 0;
        s = 0, 
        v = 255 - s;

        return;
    }
    
    // scale all channels up to compensate for desaturation
    if( s < 255) {
        if( s == 0) s = 1;
        uint32_t scaleup = 65535 / (s);
        r = ((uint32_t)(r) * scaleup) >> 8;
        g = ((uint32_t)(g) * scaleup) >> 8;
        b = ((uint32_t)(b) * scaleup) >> 8;
    }
    
    uint16_t total = r + g + b;
    

    if( total < 255) {
        if( total == 0) total = 1;
        uint32_t scaleup = 65535 / (total);
        r = ((uint32_t)(r) * scaleup) >> 8;
        g = ((uint32_t)(g) * scaleup) >> 8;
        b = ((uint32_t)(b) * scaleup) >> 8;
    }
    
    if( total > 255 ) {
        v = 255;
    } else {
        v = qadd8(desat,total);
        // undo 'dimming' of brightness
        if( v != 255) v = sqrt16(uint16_t(v) << 8);
        // without lib8tion: real_t ... ew ... sqrt... double ew, or rather, ew ^ 0.5
        // if( v != 255) v = (256.0 * sqrt( (real_t)(v) / 256.0));
        
    }
    
    uint8_t highest = r;
    if( g > highest) highest = g;
    if( b > highest) highest = b;
    
    if( highest == r ) {
        // Red is highest.
        // Hue could be Purple/Pink-Red,Red-Orange,Orange-Yellow
        if( g == 0 ) {
            // if green is zero, we're in Purple/Pink-Red
            h = (HUE_PURPLE + HUE_PINK) >> 1;
            h += scale8( qsub8(r, 128), FIXFRAC8(48,128));
        } else if ( (r - g) > g) {
            // if R-G > G then we're in Red-Orange
            h = HUE_RED;
            h += scale8( g, FIXFRAC8(32,85));
        } else {
            // R-G < G, we're in Orange-Yellow
            h = HUE_ORANGE;
            h += scale8( qsub8((g - 85) + (171 - r), 4), FIXFRAC8(32,85)); //221
        }
        
    } else if ( highest == g) {
        // Green is highest
        // Hue could be Yellow-Green, Green-Aqua
        if( b == 0) {
            // if Blue is zero, we're in Yellow-Green
            //   G = 171..255
            //   R = 171..  0
            h = HUE_YELLOW;
            uint8_t radj = scale8( qsub8(171,r),   47); //171..0 -> 0..171 -> 0..31
            uint8_t gadj = scale8( qsub8(g,171),   96); //171..255 -> 0..84 -> 0..31;
            uint8_t rgadj = radj + gadj;
            uint8_t hueadv = rgadj >> 1;
            h += hueadv;
            //h += scale8( qadd8( 4, qadd8((g - 128), (128 - r))),
            //             FIXFRAC8(32,255)); //
        } else {
            // if Blue is nonzero we're in Green-Aqua
            if( (g-b) > b) {
                h = HUE_GREEN;
                h += scale8( b, FIXFRAC8(32,85));
            } else {
                h = HUE_AQUA;
                h += scale8( qsub8(b, 85), FIXFRAC8(8,42));
            }
        }
        
    } else /* highest == b */ {
        // Blue is highest
        // Hue could be Aqua/Blue-Blue, Blue-Purple, Purple-Pink
        if( r == 0) {
            // if red is zero, we're in Aqua/Blue-Blue
            h = HUE_AQUA + ((HUE_BLUE - HUE_AQUA) >> 2);
            h += scale8( qsub8(b, 128), FIXFRAC8(24,128));
        } else if ( (b-r) > r) {
            // B-R > R, we're in Blue-Purple
            h = HUE_BLUE;
            h += scale8( r, FIXFRAC8(32,85));
        } else {
            // B-R < R, we're in Purple-Pink
            h = HUE_PURPLE;
            h += scale8( qsub8(r, 85), FIXFRAC8(32,85));
        }
    }
    
    h += 1;
}

constexpr RGB888::RGB888(const HSV888 & hsv){
    
    #define APPLY_DIMMING(X) (X)
    #define HSV_SECTION_6 (0x20)
    #define HSV_SECTION_3 (0x40)
    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves
    uint8_t value = APPLY_DIMMING( hsv.v);
    uint8_t saturation = hsv.s;

    // The brightness floor is minimum number that all of
    // R, G, and B will be set to.
    uint8_t invsat = APPLY_DIMMING( 255 - saturation);
    uint8_t brightness_floor = (value * invsat) >> 8;

    // The color amplitude is the maximum amount of R, G, and B
    // that will be added on top of the brightness_floor to
    // create the specific hue desired.
    uint8_t color_amplitude = value - brightness_floor;

    // Figure out which section of the hue wheel we're in,
    // and how far offset we are withing that section
    uint8_t section = hsv.h / HSV_SECTION_3; // 0..2
    uint8_t offset = hsv.h % HSV_SECTION_3;  // 0..63

    uint8_t rampup = offset; // 0..63
    uint8_t rampdown = (HSV_SECTION_3 - 1) - offset; // 63..0

    uint8_t rampup_amp_adj   = (rampup   * color_amplitude) >> 6;
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) >> 6;

    // add brightness_floor offset to everything
    uint8_t rampup_adj_with_floor   = rampup_amp_adj   + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;
    
    if(section) {
        if( section == 1) {
            // section 1: 0x40..0x7F
            r = brightness_floor;
            g = rampdown_adj_with_floor;
            b = rampup_adj_with_floor;
        } else {
            // section 2; 0x80..0xBF
            r = rampup_adj_with_floor;
            g = brightness_floor;
            b = rampdown_adj_with_floor;
        }
    } else {
        // section 0: 0x00..0x3F
        r = rampdown_adj_with_floor;
        g = rampup_adj_with_floor;
        b = brightness_floor;
    }

}

constexpr RGB565::RGB565(const HSV888 & hsv):RGB565(RGB888(hsv)){;}
constexpr HSV888::HSV888(const RGB565 & rgb):HSV888(RGB888(rgb)){;}




class OutputStream;


OutputStream & operator<<(OutputStream & os, const Binary & bn);

OutputStream & operator<<(OutputStream & os, const Gray & gs);

OutputStream & operator<<(OutputStream & os, const IGray & sgs);

OutputStream & operator<<(OutputStream & os, const RGB565 & rgb);

OutputStream & operator<<(OutputStream & os, const RGB888 & rgb);

OutputStream & operator<<(OutputStream & os, const LAB888 & lab);

OutputStream & operator<<(OutputStream & os, const HSV888 & hsv);

template<typename T>
concept is_monochrome = ::std::is_same_v<T, Binary> or ::std::is_same_v<T, Gray> or ::std::is_same_v<T, IGray>;

template<typename T>
concept is_rgb = ::std::is_same_v<T, RGB24> or ::std::is_same_v<T, RGB332> or ::std::is_same_v<T, RGB565> or ::std::is_same_v<T, RGB888> ;

template<typename T>
concept is_polychrome = is_rgb<T> or ::std::is_same_v<T, LAB888> or ::std::is_same_v<T, HSV888>;

template<typename T>
concept is_color = is_monochrome<T> or is_polychrome<T> or ::std::is_same_v<T, ColorEnum>;

}
